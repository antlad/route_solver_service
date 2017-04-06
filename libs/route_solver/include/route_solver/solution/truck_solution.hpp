//
// Created by vlad on 03.12.16.
//

#pragma once

#include <route_solver/structs/data_structs.hpp>
#include <route_solver/abc/abc.hpp>
#include <route_solver/utils/common.hpp>
#include <route_solver/solution/currentroutestate.hpp>
#include <route_solver/solution/fail_statistics.hpp>

#include <boost/function.hpp>
#include <Eigen/Core>

#include <vector>
#include <unordered_map>


namespace rs {

struct PointStat;

struct Route
{
	Route() = default;

	Route(Route&& o) = default;
	Route(const Route& o) = default;

	Route& operator=(const Route& o) = default;

	std::int32_t vehicleId = 0;
	bool solved = false;
	std::vector<std::int32_t> nodes;

	bool operator== (const Route& o) const
	{
		return nodes == o.nodes && vehicleId == o.vehicleId;
	}
};

}

namespace std {
template<> struct hash<rs::Route>
{
	std::size_t  operator()(rs::Route const& r) const
	{
		std::size_t seed = boost::hash_range(r.nodes.begin(), r.nodes.end());
		boost::hash_combine(seed, r.vehicleId);
		return seed;
	}
};
}

namespace rs {

struct RoutesCheckResult
{
	double fitness = 0;
	double totalDistance = 0;
	std::size_t totalTime = 0;
	bool allSolved = false;
};

class TruckSolution
	: public rs::Solution<TruckSolution> {
public:
	TruckSolution(
		std::vector<Route>&& routes,
		RoutesCheckResult&& stat
	)
		: m_stat(std::move(stat))
		, m_routes(std::move(routes))
	{}

	double fitnessImpl() const;

	const std::vector<Route>& getRoutes() const;

	const RoutesCheckResult& getStat() const;

private:
	RoutesCheckResult m_stat;
	std::vector<Route> m_routes;
};


struct TaskWithStat
{
	std::uint32_t nodeId;
	std::size_t timeStart;
	std::size_t timeEnd;
	double workloadDiff;
	double volumeDiff;
	double distanceDiff;
	int tasksCountDiff;
};

struct RouteWithStat
{
	double length;
	std::uint32_t vehicleId;
	std::size_t timeStart;
	std::size_t timeEnd;
	std::vector<TaskWithStat> tasks;

};

struct ProdSolutionWithStat
{
	std::vector<RouteWithStat> routes;
};


class TruckSolutionProducer
	: public rs::SolutionProducer<TruckSolutionProducer, TruckSolution> {
public:
	TruckSolutionProducer(
		const Eigen::MatrixXd& destTimeTable,
		const Eigen::MatrixXd& destTable,
		const Eigen::MatrixXd& vehicleToTaskValid,
		const std::vector<rs::Vehicle>& vehicles,
		const std::vector<rs::Task>& tasks,
		bool partialSolutionResultEnabled
	);

	TruckSolution generateRandomImpl();

	TruckSolution produceNeighborhoodImpl(const TruckSolution& solution);

	ProdSolutionWithStat getResultStats(const TruckSolution& solution);

public:
	struct SingleRouteStatistics
	{
		double cost = std::numeric_limits<double>::max();
		double distanse = 0;
		std::size_t totalTime = 0;
		std::size_t vipCount = 0;
	};

	void mergeFails();

	static FailedStatisticsCollector getFailsCollector();

private:

	RoutesCheckResult calculateFitnessOfRoutes(std::vector<Route>& routes);

	SingleRouteStatistics checkSingleRoute(
		Route& r,
		std::vector<PointStat>* stats = 0
	);

	const Eigen::MatrixXd& m_destTimeTable;
	const Eigen::MatrixXd& m_destTable;
	const Eigen::MatrixXd& m_vehicleToTaskValid;
	const std::vector<rs::Vehicle>& m_vehicles;
	const std::vector<rs::Task>& m_tasks;
	bool m_partialSolutionResultEnabled;

	std::unique_ptr<std::random_device> m_rd;
	std::unique_ptr<std::mt19937> m_g;
	std::unordered_map<Route, SingleRouteStatistics> m_routes_cache;

	FailedStatisticsCollector m_failsCollector;
};

}
