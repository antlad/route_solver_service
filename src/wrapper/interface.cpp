#include "wrapper/interface.hpp"
#include "solve/solver.hpp"

#include <route_solver/log.hpp>
#include <route_solver/utils/common.hpp>

#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <memory>
#include <mutex>
#include <chrono>

namespace bf = boost::filesystem;

static std::unique_ptr<rto::Solver> solver;

int setup(const char* settings)
{
	try
	{
		rs::log::initializeWithCallBack([](const char* msg)
		{
			logCallback(msg);
		});
		//		rto::log::initializeJustConsole();
		LOG_DEBUG << "setup library with json " << settings;
		auto j = nlohmann::json::parse(settings);
		std::string osrm_path = j["osrm_path"];

		if (!bf::exists(osrm_path))
		{
			const auto& s = (boost::format("File path %s doesn't exists") % osrm_path).str();
			BOOST_THROW_EXCEPTION(std::runtime_error(s));
		}

		solver.reset(new rto::Solver(osrm_path));
	}
	catch (const std::exception& e)
	{
		std::cerr << boost::diagnostic_information(e);
		return 1;
	}

	return 0;
}


int processRequest(void* ctx, const char* json)
{
	try
	{
		LOG_INFO << "start processing request";
		rs::utils::TimerElapsed timer;
		std::size_t now = std::chrono::seconds(std::chrono::seconds(std::time(0))).count();
		LOG_INFO << "start parsing json";
		auto p = rto::JsonParser::fromString(std::string(json));
		rs::log::setLogLevelFromString(p.getLogLevel());
		LOG_INFO << "done parsing json";
		auto results = solver->solve(p.getVenchiles(), p.getZones(), p.getTasks(), p.getSpeedAprox(), p.getPartialSolutionEnabled(), p.getMultiThreaded(), p.getAbcParams());
		nlohmann::json out;
		out["account_id"] = p.getAccountId();
		out["calculation_time"] = int((double)timer.elapsedMiliseconds() / 1000.0);
		out["calculation_start"] = now;

		for (const auto& r : results.routes)
		{
			nlohmann::json route;
			route["vehicle"] = r.vehicle;
			route["ts"] = r.startTime;
			route["te"] = r.stopTime;
			route["length"] = r.distance;
			route["polyline"] = r.polyline;

			for (const auto& t : r.tasks)
			{
				nlohmann::json task;
				task["id"] = t.id;
				task["ts"] =  t.startTime;
				task["te"] = t.stopTime;
				task["workload"] = t.workload;
				task["volume"] = t.volume;
				task["distance"] = t.distance;
				task["active_tasks"] = t.tasksCount;

				switch (t.action)
				{
				case rto::Load:
				{
					task["action"] = "Load";
					break;
				}

				case rto::Unload:
				{
					task["action"] = "Unload";
					break;
				}

				default:
					BOOST_THROW_EXCEPTION(std::runtime_error("Unknown action, internal error"));
				}

				route["tasks"].push_back(task);
			}

			out["trips"].push_back(route);
		}

		auto out_str = out.dump();
		LOG_DEBUG << "result solution is:\n " << out_str;
		processRequestDone(ctx, out_str.c_str());
	}
	catch (const std::exception& e)
	{
		processRequestError(ctx, boost::diagnostic_information(e).c_str());
		rs::log::doLoggerFlush();
		return 1;
	}
	catch (...)
	{
		processRequestError(ctx, "Unkonwn exception");
		rs::log::doLoggerFlush();
		return 1;
	}

	return 0;
}
