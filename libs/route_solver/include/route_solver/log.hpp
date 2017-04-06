#pragma once

#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/thread/thread.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace rs {

namespace log {

enum LogLevel
{
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

typedef boost::log::sources::severity_logger_mt<LogLevel> logger_t;

typedef std::function<void (const char* msg)> LogFuncCallBack;

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", LogLevel)

BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::log::attributes::local_clock::value_type)

BOOST_LOG_ATTRIBUTE_KEYWORD(threadid, "ThreadID", boost::log::attributes::current_thread_id::value_type)

BOOST_LOG_ATTRIBUTE_KEYWORD(request_id, "RequestID", std::string)

logger_t& getLogRef();

void setLogLevelFromString(const std::string& level);

void setLogLevel(LogLevel level);

void initializeJustConsole();

void initializeWithCallBack(const LogFuncCallBack& func);

void doLoggerFlush();

void initializeLogger(const std::string& loggingFolder);
}

}

#define LOG_TRACE     BOOST_LOG_STREAM_SEV(rs::log::getLogRef(), rs::log::Trace) \
        << __FUNCTION__ << ":" << __LINE__ << "(): "

#define LOG_DEBUG     BOOST_LOG_STREAM_SEV(rs::log::getLogRef(), rs::log::Debug) \
        << __FUNCTION__ << ":" << __LINE__ << "(): "

#define LOG_INFO      BOOST_LOG_STREAM_SEV(rs::log::getLogRef(), rs::log::Info) \
        << __FUNCTION__ << ":" << __LINE__ << "(): "

#define LOG_WARNING   BOOST_LOG_STREAM_SEV(rs::log::getLogRef(), rs::log::Warning) \
        << __FUNCTION__ << ":" << __LINE__ << "(): "

#define LOG_ERROR     BOOST_LOG_STREAM_SEV(rs::log::getLogRef(), rs::log::Error) \
        << __FUNCTION__ << ":" << __LINE__ << "(): "

#define LOG_FATAL     BOOST_LOG_STREAM_SEV(rs::log::getLogRef(), rs::log::Fatal) \
        << __FUNCTION__ << ":" << __LINE__ << "(): "

#define LOG           LOG_INFO
