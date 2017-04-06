#include "solve/solver.hpp"

#include <route_solver/solve/vehicle_to_task_checker.hpp>
#include <route_solver/abc/abc.hpp>
#include <route_solver/abc/abc_parallel.hpp>
#include <route_solver/log.hpp>
#include <route_solver/utils/common.hpp>
#include <route_solver/solution/truck_solution.hpp>

#include <osrm/route_parameters.hpp>
#include <osrm/table_parameters.hpp>
#include <osrm/engine_config.hpp>
#include <osrm/engine/api/base_parameters.hpp>
#include <osrm/json_container.hpp>
#include <osrm/osrm.hpp>
#include <osrm/util/coordinate.hpp>

#include <boost/format.hpp>
#include <Eigen/Core>

#include <fstream>

namespace rto {

using namespace osrm;

class SolverPrivate {
public:
	SolverPrivate(const std::string& osrm_data_path)
	{
		m_engineConfig.storage_config = {osrm_data_path};
		m_engineConfig.use_shared_memory = false;
		m_osrm.reset(new OSRM(m_engineConfig));
	}

	~SolverPrivate()
	{}

	EngineConfig m_engineConfig;
	bool m_produceDebugFiles;
	std::unique_ptr<OSRM> m_osrm;
};

Solver::Solver(const std::string& osrm_data_path)
	: p(new SolverPrivate(osrm_data_path))
{
}

Solver::~Solver()
{
}

namespace  {
void checkStatus(const osrm::Status& status, json::Object& result)
{
	if (status != Status::Ok)
	{
		const auto code = result.values["code"].get<json::String>().value;
		const auto message = result.values["message"].get<json::String>().value;
		auto msg = (boost::format("Error during table calculation code: %s , message: %s") % code % message).str();
		BOOST_THROW_EXCEPTION(std::runtime_error(msg));
	}
}
}

struct ResponseVisitor
{
	ResponseVisitor(int tabs = 0)
		: tabs(tabs)
	{}

	void operator()(mapbox::util::recursive_wrapper<json::Object> o) const
	{
		print_object(o.get());
	}

	void print_object(const json::Object& o) const
	{
		ResponseVisitor rsp_vis(tabs + 1);

		for (const auto& a : o.values)
		{
			LOG_TRACE << std::string(tabs * 4, ' ')  << a.first;
			mapbox::util::apply_visitor(rsp_vis, a.second);
		}
	}

	void operator()(mapbox::util::recursive_wrapper<json::Array> arr) const
	{
		ResponseVisitor rsp_vis(tabs + 1);

		for (const auto& a : arr.get().values)
		{
			LOG_TRACE << std::string(tabs * 4, ' ') << "---";
			mapbox::util::apply_visitor(rsp_vis, a);
		}
	}

	void operator()(json::Number n) const
	{
		LOG_TRACE << std::string(tabs * 4, ' ') << n.value;
	}

	void operator()(json::True) const
	{
		LOG_TRACE << std::string(tabs * 4, ' ') << "true";
	}

	void operator()(json::False) const
	{
		LOG_TRACE << std::string(tabs * 4, ' ') << "false";
	}

	void operator()(json::Null) const
	{
		LOG_TRACE << std::string(tabs * 4, ' ') << "null";
	}

	void operator()(json::String s) const
	{
		LOG_TRACE << std::string(tabs * 4, ' ') << s.value;
	}

	int tabs;
};



TotalResult Solver::solve(const std::vector<rs::Vehicle>& vehicles,
						  const std::vector<rs::Zone>& zones,
						  const std::vector<rs::Task>& tasks,
						  double speedForAproximationMetersInSecond,
						  bool partialSolutionEnabled,
						  bool multiThreaded,
						  const ABCParams& abcParams)
{
	rs::utils::TimerElapsed timer;
	LOG_DEBUG << "Prepare data for solve";
	rs::VehicleTaskChecker checker(vehicles, zones, tasks);
	TableParameters tableParams;
	const auto add_point = [&tableParams](double lon, double lat)
	{
		tableParams.coordinates.push_back({util::FloatLongitude{lon}, util::FloatLatitude{lat}});
		tableParams.sources.push_back(tableParams.coordinates.size() - 1);
		tableParams.destinations.push_back(tableParams.coordinates.size() - 1);
	};
	std::stringstream ss;

	for (const auto& e : tasks)
	{
		double lon = e.getStart().point.lon;
		double lat = e.getStart().point.lat;
		add_point(lon, lat);
		ss << "(" << lat << "," << lon << ") ";
		lon = e.getEnd().point.lon;
		lat = e.getEnd().point.lat;
		ss << "(" << lat << "," << lon << ") ";
		add_point(e.getEnd().point.lon, e.getEnd().point.lat);
	}

	LOG_TRACE << "Tasks points in matrix " << ss .str();
	LOG_DEBUG << "Calling engine to get time table from osrm";
	json::Object result;
	auto status = p->m_osrm->Table(tableParams, result);
	checkStatus(status, result);
	LOG_DEBUG << "Time table done in " << timer.elapsedMiliseconds() << " ms";
	Eigen::MatrixXd timeMatrix(tableParams.sources.size(), tableParams.destinations.size());
	Eigen::MatrixXd distanceMatrix(tableParams.sources.size(), tableParams.destinations.size());

	for (const auto e : result.values)
	{
		if (e.first == "durations")
		{
			int i = 0;

			for (const auto& a : e.second.get<json::Array>().values)
			{
				int j = 0;

				for (const auto& f : a.get<json::Array>().values)
				{
					auto timeInSeconds = f.get<json::Number>().value;
					timeMatrix(i, j) = timeInSeconds;
					distanceMatrix(i, j) = timeInSeconds * speedForAproximationMetersInSecond;
					j++;
				}

				++i;
			}
		}
	}

	LOG_DEBUG << "Start getting distance for every node";
	LOG_DEBUG << "Possibility matrix vehicle to task\n " << checker.possibilityVehicleToTask();
	std::map<std::uint32_t, std::uint32_t> vehiclesToExtenal;

	for (const auto& v : vehicles)
	{
		vehiclesToExtenal[v.getId()] = v.getExternalId();
	}

	LOG_TRACE << "\n" << timeMatrix;
	LOG_TRACE << "\n" << distanceMatrix;
	;
	rs::Statistics stat;
	rs::BeeColonyParams beeColonyParams(abcParams.colonySize,
										abcParams.trialCountMax,
										abcParams.maxTimeInSecHardLimit,
										abcParams.maxTimeInSecNoNewBestSolution);
	auto producerFabric = [&]()
	{
		return rs::TruckSolutionProducer(
				   timeMatrix,
				   distanceMatrix,
				   checker.possibilityVehicleToTask(),
				   vehicles,
				   tasks,
				   partialSolutionEnabled
			   );
	};
	auto helper = producerFabric();
	auto solution = helper.generateRandom();

	if (multiThreaded)
	{
		solution = rs::beeColonySolveParallel<rs::TruckSolutionProducer, rs::TruckSolution>(
					   producerFabric,
					   beeColonyParams,
					   &stat
				   );
	}
	else
	{
		solution = rs::beeColonySolve<rs::TruckSolutionProducer, rs::TruckSolution>(
					   producerFabric,
					   beeColonyParams,
					   &stat
				   );
	}

	LOG_TRACE << rs::TruckSolutionProducer::getFailsCollector().print();

	if (!solution.getStat().allSolved)
	{
		LOG_WARNING << "Not all tasks solved";
	}

	bool any_solved = false;

	for (const auto& r : solution.getRoutes())
	{
		if (r.solved)
		{
			any_solved = true;
			break;
		}
	}

	if (!any_solved)
	{
		throw std::runtime_error("No solution found!");
	}

	auto producer = producerFabric();
	auto resultWithStats = producer.getResultStats(solution);
	TotalResult results;
	results.routes.reserve(solution.getRoutes().size());

	// TODO: refactor this ugly code:
	for (const auto& r : resultWithStats.routes)
	{
		ResultRoute route;
		route.vehicle = vehiclesToExtenal[r.vehicleId];
		LOG_TRACE << "vehicle: " << route.vehicle;
		route.tasks.reserve(r.tasks.size());
		route.startTime = r.timeStart;
		LOG_TRACE << "startTime: " << route.startTime;
		route.stopTime = r.timeEnd;
		LOG_TRACE << "stopTime: " << route.stopTime;
		LOG_TRACE << "tasks:";
		double workloadTotal = 0;
		double volumeTotal = 0;
		double distanceTotal = 0;
		int tasksCountTotal = 0;
		osrm::RouteParameters routeParams;
		routeParams.overview = osrm::engine::api::RouteParameters::OverviewType::Full;
		std::stringstream ss_csv;
		ss_csv << "lat,lon,name" << std::endl;

		for (auto n : r.tasks)
		{
			distanceTotal += n.distanceDiff;
			tasksCountTotal += n.tasksCountDiff;
			volumeTotal += n.volumeDiff;
			workloadTotal += n.workloadDiff;
			ResultTask task;
			// TODO: remove this copy paste
			task.startTime = n.timeStart;
			task.stopTime = n.timeEnd;
			task.distance = distanceTotal;
			task.tasksCount = tasksCountTotal;
			task.volume = volumeTotal;
			task.workload = workloadTotal;
			const auto& o_task = tasks[n.nodeId / 2];
			task.id = o_task.getExternalId();

			if (n.nodeId % 2 == 0)
			{
				task.action = rto::Load;
			}
			else
			{
				task.action = rto::Unload;
			}

			const auto& pt = tableParams.coordinates[n.nodeId];
			auto f_lat = osrm::util::toFloating(pt.lat);
			auto f_lon = osrm::util::toFloating(pt.lon);
			routeParams.coordinates.push_back(pt);
			LOG_TRACE << "	point:";
			LOG_TRACE << "		id: " << task.id;
			ss_csv << f_lat << "," << f_lon << ",node_" << n.nodeId << std::endl;

			if (task.action == rto::Load)
			{
				LOG_TRACE << "		action Load";
			}
			else
			{
				LOG_TRACE << "		action Unload";
			}

			LOG_TRACE << "		startTime " << task.startTime;
			LOG_TRACE << "		stopTime " << task.stopTime;
			LOG_TRACE << "		workload " << task.workload;
			LOG_TRACE << "		volume " << task.volume;
			LOG_TRACE << "		distance " << task.distance;
			LOG_TRACE << "		total tasks " << task.tasksCount;
			route.tasks.push_back(task);
		}

		LOG_TRACE << "Getting polyline from osrm";
		json::Object route_result;
		auto status = p->m_osrm->Route(routeParams, route_result);
		checkStatus(status, route_result);
		auto& osrm_routes = route_result.values["routes"].get<json::Array>();
		auto& osrm_route = osrm_routes.values.at(0).get<json::Object>();
		route.distance = distanceTotal;//osrm_route.values["distance"].get<json::Number>().value;
		route.polyline = osrm_route.values["geometry"].get<json::String>().value;
		const auto duration = osrm_route.values["duration"].get<json::Number>().value;
		LOG_TRACE << "		osrm duration " << duration;
		LOG_TRACE << "		osrm distance " << route.distance;
		LOG_TRACE << "		osrm geometry " << route.polyline;
		LOG_TRACE << "csv output\nsave this to file and use http://www.gpsvisualizer.com/draw/\n" << ss_csv.str();
		results.routes.push_back(route);
	}

	return results;
}


}
