//
// Created by vlad on 03.12.16.
//

#include "route_solver/solution/truck_solution.hpp"
#include "route_solver/solution/currentroutestate.hpp"
#include <algorithm>
#include <iterator>
#include <random>

namespace rs {

TruckSolutionProducer::TruckSolutionProducer(
	const Eigen::MatrixXd& destTimeTable,
	const Eigen::MatrixXd& destTable,
	const Eigen::MatrixXd& vehicleToTaskValid,
	const std::vector<rs::Vehicle>& vehicles,
	const std::vector<rs::Task>& tasks,
	bool partialSolutionResultEnabled
)
	: m_destTimeTable(destTimeTable)
	, m_destTable(destTable)
	, m_vehicleToTaskValid(vehicleToTaskValid)
	, m_vehicles(vehicles)
	, m_tasks(tasks)
	, m_partialSolutionResultEnabled(partialSolutionResultEnabled)
	, m_rd(new std::random_device())
	, m_g(new std::mt19937(m_rd->operator()()))
{
}



TruckSolution TruckSolutionProducer::generateRandomImpl()
{
	std::uniform_int_distribution<> dis_vehicles_num(1, m_vehicles.size());
	auto vehiclesNum = dis_vehicles_num(*m_g);
	std::vector<Route> routes(vehiclesNum);
	std::uint32_t vehiclesIds[m_vehicles.size()];
	memset(vehiclesIds, 0, m_vehicles.size() * sizeof(std::uint32_t));
	auto* ptr = &vehiclesIds[0];

	for (auto& e : m_vehicles)
	{
		*ptr++ = e.getId();
	}

	std::shuffle(&vehiclesIds[0], &vehiclesIds[m_vehicles.size()], *m_g);
	const auto* cptr = &vehiclesIds[0];

	for (auto& r : routes)
	{
		r.nodes.reserve(m_tasks.size() * 2);
		r.vehicleId = *cptr++;
	}

	std::size_t taskIdShuf[m_tasks.size()];

	for (std::size_t i = 0; i < m_tasks.size(); ++i)
	{
		taskIdShuf[i] = i;
	}

	std::size_t tasksNum = 0;

	if (m_partialSolutionResultEnabled)
	{
		std::uniform_int_distribution<> dis_tasks_num(1, m_tasks.size());
		tasksNum = dis_tasks_num(*m_g);
	}
	else
	{
		tasksNum = m_tasks.size();
	}

	std::shuffle(&taskIdShuf[0], &taskIdShuf[m_tasks.size()], *m_g);
	std::uniform_int_distribution<> nextVehicleDis(0, vehiclesNum - 1);

	for (std::size_t i = 0; i < tasksNum; ++i)
	{
		std::size_t taskId = taskIdShuf[i];
		std::int32_t nextVehileIndex = nextVehicleDis(*m_g);
		routes[nextVehileIndex].nodes.push_back(taskId * 2);
		routes[nextVehileIndex].nodes.push_back(taskId * 2 + 1);
	}

	auto it = std::remove_if(routes.begin(), routes.end(), [](const auto & r)
	{
		return r.nodes.empty();
	});
	routes.erase(it, routes.end());

	for (auto& r : routes)
	{
		std::shuffle(r.nodes.begin(), r.nodes.end(), *m_g);
	}

	// TODO: check if we already checked this
	auto s = calculateFitnessOfRoutes(routes);
	return TruckSolution(std::move(routes), std::move(s));
}

TruckSolution TruckSolutionProducer::produceNeighborhoodImpl(const TruckSolution& solution)
{
	auto newRoutes = solution.getRoutes();
	std::uniform_int_distribution<> routes_dis(0, newRoutes.size() - 1);
	std::size_t shuffleIndex = 0;

	if (solution.getStat().allSolved)
	{
		shuffleIndex = routes_dis(*m_g);
	}
	else
	{
		for (; shuffleIndex < newRoutes.size(); ++shuffleIndex)
		{
			if (!newRoutes[shuffleIndex].solved)
			{
				break;
			}
		}
	}

	// TODO: make intellectual check which vehicle route
	auto& r = newRoutes[shuffleIndex];

	if (solution.getStat().allSolved)
	{
		std::uniform_int_distribution<> nodeIdRandom(0, r.nodes.size() - 1);
		std::int32_t i = nodeIdRandom(*m_g);
		std::swap(r.nodes[i], r.nodes[r.nodes.size() - 1 - i]);
	}
	else
	{
		std::shuffle(r.nodes.begin(), r.nodes.end(), *m_g);
	}

	auto s = calculateFitnessOfRoutes(newRoutes);
	return TruckSolution(std::move(newRoutes), std::move(s));
}

ProdSolutionWithStat TruckSolutionProducer::getResultStats(const TruckSolution& solution)
{
	ProdSolutionWithStat result;

	// TODO: rewrite this ugly code
	for (const auto& r : solution.getRoutes())
	{
		if (!r.solved)
		{
			continue;
		}

		RouteWithStat routeWithStat;
		auto routeCalculation = r;
		std::vector<PointStat> stats;
		auto stat = checkSingleRoute(routeCalculation, &stats);
		routeWithStat.length = stat.distanse;
		routeWithStat.vehicleId = routeCalculation.vehicleId;
		LOG_DEBUG << "timings is " << stats.size() << " nodes size " << r.nodes.size();

		if (stats.size() != (r.nodes.size() * 2))
		{
			BOOST_THROW_EXCEPTION(std::runtime_error("unexpected times size != nodes size !"));
		}

		routeWithStat.tasks.reserve(r.nodes.size());
		routeWithStat.timeStart = std::numeric_limits<int>::max();
		routeWithStat.timeEnd = 0;

		for (std::size_t i = 0; i < r.nodes.size(); ++i)
		{
			TaskWithStat task;
			const auto& p1 = stats[i * 2];
			const auto& p2 = stats[i * 2 + 1];
			task.nodeId = r.nodes[i];
			task.timeStart = p1.time;
			task.timeEnd = p2.time;
			task.workloadDiff = p2.workload - p1.workload;
			task.volumeDiff = p2.volume - p1.volume;
			task.distanceDiff = p1.distance - p2.distance;
			task.tasksCountDiff = p2.tasksCount - p1.tasksCount;

			if (task.timeStart < routeWithStat.timeStart)
			{
				routeWithStat.timeStart = task.timeStart;
			}

			if (task.timeEnd > routeWithStat.timeEnd)
			{
				routeWithStat.timeEnd = task.timeEnd;
			}

			routeWithStat.tasks.push_back(task);
		}

		result.routes.push_back(routeWithStat);
	}

	return result;
}

static FailedStatisticsCollector globalFailsCollector;

void TruckSolutionProducer::mergeFails()
{
	globalFailsCollector += m_failsCollector;
}

FailedStatisticsCollector TruckSolutionProducer::getFailsCollector()
{
	return globalFailsCollector;
}

RoutesCheckResult TruckSolutionProducer::calculateFitnessOfRoutes(std::vector<Route>& routes)
{
	std::int32_t tasks_solved = 0;
	double totalCost = 1.0;
	bool allSolved = true;
	double totalDistance = 0;
	std::size_t totalTime = 0;
	int totalVipCount = 0;

	for (auto& r : routes)
	{
		auto s = checkSingleRoute(r);

		if (r.solved)
		{
			tasks_solved += r.nodes.size() / 2;
			totalCost += s.cost;
			totalDistance += s.distanse;
			totalTime += s.totalTime;
			totalVipCount += s.vipCount;
		}
		else
		{
			allSolved = false;
		}
	}

	double fitness =
		1.0 / totalCost + // minimizing cost
		(double)tasks_solved / m_tasks.size() + // maximazing tasks solved
		(double)totalVipCount / m_tasks.size(); // maximazing vip tasks solved
	return RoutesCheckResult{fitness, totalDistance, totalTime, allSolved};
}

typedef TruckSolutionProducer::SingleRouteStatistics Stat;


TruckSolutionProducer::SingleRouteStatistics TruckSolutionProducer::checkSingleRoute(
	Route& r,
	std::vector<PointStat>* stats
)
{
	if (!stats)
	{
		auto it = m_routes_cache.find(r);

		if (it != m_routes_cache.end())
		{
			return it->second;
		}
	}

	std::uint32_t tasks_taken[m_tasks.size()];
	memset(tasks_taken, 0, m_tasks.size() * sizeof(std::uint32_t));

	if (r.nodes.empty())
	{
		m_failsCollector.addEvent(RouteNodesEmpty);
		return Stat();
	}

	for (auto n : r.nodes)
	{
		std::uint32_t taskId = n / 2;
		bool isTaskStart = ((n % 2) == 0);

		if (isTaskStart)
		{
			tasks_taken[taskId] = 1;
		}
		else
		{
			if (tasks_taken[taskId] != 1)
			{
				m_failsCollector.addEvent(TaskUnLoadBeforeLoad);
				return Stat();//task not taken
			}
		}
	}

	r.solved = false;
	std::size_t totalVipCountInRoute = 0;
	const auto& V = m_vehicles[r.vehicleId];

	for (auto n : r.nodes)
	{
		auto taskId = n / 2;

		if (m_vehicleToTaskValid(V.getId(), taskId) == 0)
		{
			m_failsCollector.addEvent(ImpossibleTaskToVenchile);
			return Stat();
		}
	}

	CurrentRouteState state(V, m_destTimeTable, m_destTable, m_failsCollector, stats);

	for (const auto& n : r.nodes)
	{
		std::int32_t taskId = n / 2;
		const auto& task = m_tasks[taskId];

		if (task.getVip())
		{
			++totalVipCountInRoute;
		}

		bool isTaskStart = ((n % 2) == 0);

		if (isTaskStart)
		{
			state.moveNextStart(n, task);

			if (!state.processLoad(task))
			{
				return Stat();
			}
		}
		else
		{
			if (!state.moveNextEnd(n, task))
			{
				return Stat();
			}

			if (!state.processUnload(task))
			{
				return Stat();
			}
		}
	}

	r.solved = true;
	auto result = Stat{state.totalCostOfRoute(), state.totalDistance(), state.totalTimeSpend(), totalVipCountInRoute};
	m_routes_cache.insert(std::make_pair(r, result));
	return result;
}

double TruckSolution::fitnessImpl() const
{
	return m_stat.fitness;
}

const std::vector<Route>& TruckSolution::getRoutes() const
{
	return m_routes;
}

const RoutesCheckResult& TruckSolution::getStat() const
{
	return m_stat;
}

}
