#include "route_solver/log.hpp"

#include <boost/phoenix/bind/bind_function_object.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_multifile_backend.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace rs {

namespace log {

namespace expr = boost::log::expressions;

typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_multifile_backend> SinkType;

LogLevel gLogLevel = Info;

logger_t gLogger;

bool log_level_filter(boost::log::value_ref<LogLevel, tag::severity> const& level)
{
	return level >= gLogLevel;
}

class SinkWithCallBack :
	public boost::log::sinks::basic_formatted_sink_backend <
	char,
	boost::log::sinks::synchronized_feeding
	> {
public:
	SinkWithCallBack(const LogFuncCallBack& func)
		: m_func(func)
	{
	}

	void consume(boost::log::record_view const& rec, string_type const& line)
	{
		m_func(line.c_str());
	}

private:
	LogFuncCallBack m_func;
};

typedef boost::log::sinks::synchronous_sink<SinkWithCallBack> SinkWithCallBackType;

void setLogLevelFromString(const std::string& level)
{
	//, "debug", " info", " warn", "error", "fatal"
	if (level == "trace")
	{
		setLogLevel(Trace);
	}
	else if (level == "debug")
	{
		setLogLevel(Debug);
	}
	else if (level == "info")
	{
		setLogLevel(Info);
	}
	else if (level == "warn")
	{
		setLogLevel(Warning);
	}
	else if (level == "error")
	{
		setLogLevel(Error);
	}
	else if (level == "fatal")
	{
		setLogLevel(Fatal);
	}
	else
	{
		BOOST_THROW_EXCEPTION(std::runtime_error("Unsupported logging level"));
	}
}

void setLogLevel(LogLevel level)
{
	gLogLevel = level;
}

logger_t& getLogRef()
{
	return gLogger;
}

std::ostream& operator<<(std::ostream& stream, LogLevel level)
{
	static const char* strings[] = {"trace", "debug", " info", " warn", "error", "fatal"};
	int l = static_cast<size_t>(level);

	if (l >= 0 && l < sizeof(strings) / sizeof(strings[0]))
	{
		stream << strings[l];
	}
	else
	{
		stream << l;
	}

	return stream;
}

void initializeJustConsole()
{
	auto sink = boost::make_shared<SinkType>();
	sink->set_filter(boost::phoenix::bind(&log_level_filter, severity.or_none()));
	auto consoleSink = boost::log::add_console_log(std::clog);
	consoleSink->set_formatter(boost::log::expressions::stream
							   << "[" << expr::attr<boost::thread::id>("ThreadID") << "]"
							   << "[" << boost::log::expressions::format_date_time(
								   timestamp, "%Y-%m-%d %H:%M:%S") << "]"
							   << "[" << severity << "] "
							   << "[" << request_id << "] "
							   << boost::log::expressions::smessage);
	boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
}

void doLoggerFlush()
{
	boost::log::core::get()->flush();
}

void initializeLogger(const std::string& loggingFolder)
{
	auto sink = boost::make_shared<SinkType>();
	sink->set_filter(boost::phoenix::bind(&log_level_filter, severity.or_none()));
	auto loggingFolder_ = loggingFolder;

	if (loggingFolder.empty() || loggingFolder_.back() != '/')
	{
		loggingFolder_.push_back('/');
	}

	sink->locked_backend()->set_file_name_composer(boost::log::sinks::file::as_file_name_composer(
				expr::stream << loggingFolder_ << expr::attr<std::string>("RequestID") << ".log"));
	sink->set_formatter(boost::log::expressions::stream
						<< "[" << expr::attr<boost::thread::id>("ThreadID") << "]"
						<< "[" << boost::log::expressions::format_date_time(
							timestamp, "%Y-%m-%d %H:%M:%S") << "]"
						<< "[" << severity << "] "
						<< "[" << request_id << "] "
						<< boost::log::expressions::smessage);
	boost::log::core::get()->add_sink(sink);
	auto consoleSink = boost::log::add_console_log(std::clog);
	consoleSink->set_formatter(boost::log::expressions::stream
							   << "[" << expr::attr<boost::thread::id>("ThreadID") << "]"
							   << "[" << boost::log::expressions::format_date_time(
								   timestamp, "%Y-%m-%d %H:%M:%S") << "]"
							   << "[" << severity << "] "
							   << "[" << request_id << "] "
							   << boost::log::expressions::smessage);
	consoleSink->set_filter(boost::phoenix::bind(&log_level_filter, severity.or_none()));
	boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
}

void initializeWithCallBack(const LogFuncCallBack& func)
{
	auto sink = boost::make_shared<SinkWithCallBackType>(func);
	sink->set_filter(boost::phoenix::bind(&log_level_filter, severity.or_none()));
	auto consoleSink = boost::log::add_console_log(std::clog);
	consoleSink->set_formatter(boost::log::expressions::stream
							   << "[" << expr::attr<boost::thread::id>("ThreadID") << "]"
							   << "[" << boost::log::expressions::format_date_time(
								   timestamp, "%Y-%m-%d %H:%M:%S") << "]"
							   << "[" << severity << "] "
							   << "[" << request_id << "] "
							   << boost::log::expressions::smessage);
	boost::log::core::get()->add_global_attribute("TimeStamp", boost::log::attributes::local_clock());
}

}
}
