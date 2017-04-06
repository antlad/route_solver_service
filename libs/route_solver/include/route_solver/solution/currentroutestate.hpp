#pragma once

#include <route_solver/structs/data_structs.hpp>
#include <route_solver/solution/fail_statistics.hpp>
#include <Eigen/Core>

namespace rs {

struct PointStat
{
	PointStat(
		std::uint32_t nodeId,
		std::size_t time,
		double workload,
		double volume,
		double distance,
		int tasksCount
	);

	std::uint32_t nodeId;
	std::size_t time;
	double workload;
	double volume;
	double distance;
	int tasksCount;
};




class CurrentRouteState {
public:

	CurrentRouteState(
		const Vehicle& vehicle,
		const Eigen::MatrixXd& destTimeTable,
		const Eigen::MatrixXd& destTable,
		FailedStatisticsCollector& failsCollector,
		std::vector<PointStat>* stats = 0);

	void moveNextStart(std::int32_t nextNode, const Task& task);

	bool processLoad(const Task& task);

	bool processUnload(const Task& task);

	bool moveNextEnd(std::int32_t nextNode, const Task& task);

	std::size_t totalTimeSpend() const;

	double totalDistance() const;

	double totalCostOfRoute() const;

	void pushStatsIfNeeded(double nodeDistance);

private:
	const Vehicle& m_vehicle;
	const Eigen::MatrixXd& m_destTimeTable;
	const Eigen::MatrixXd& m_destTable;

	std::size_t m_currentTime = 0;
	double m_currentDistanceMeters = 0;
	std::size_t m_routeStartTime = 0;
	double m_currentLoad = 0;
	double m_currentVolume = 0;

	std::uint32_t m_taskTaken = 0;
	std::int32_t m_currentNode = -1;
	std::vector<PointStat>* m_stats;
	FailedStatisticsCollector& m_failsCollector;
};

}
