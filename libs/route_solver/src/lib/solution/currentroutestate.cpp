#include "route_solver/solution/currentroutestate.hpp"

namespace rs {

CurrentRouteState::CurrentRouteState(
	const Vehicle& vehicle,
	const Eigen::MatrixXd& destTimeTable,
	const Eigen::MatrixXd& destTable,
	FailedStatisticsCollector& failsCollector,
	std::vector<PointStat>* stats
)
	: m_vehicle(vehicle)
	, m_destTimeTable(destTimeTable)
	, m_destTable(destTable)
	, m_stats(stats)
	, m_failsCollector(failsCollector)
{
}

inline void CurrentRouteState::pushStatsIfNeeded(double nodeDistance)
{
	if (m_stats)
	{
		m_stats->emplace_back(
			m_currentNode,
			m_currentTime,
			m_currentLoad,
			m_currentVolume,
			nodeDistance,
			m_taskTaken
		);
	}
}


void CurrentRouteState::moveNextStart(int32_t nextNode, const Task& task)
{
	double nodeDistance = 0;

	if (m_currentNode == -1)
	{
		m_currentTime = task.getStart().ts;
		m_routeStartTime = m_currentTime;
	}
	else
	{
		m_currentTime += m_destTimeTable(m_currentNode, nextNode);
		nodeDistance = m_destTable(m_currentNode, nextNode);
		m_currentDistanceMeters += nodeDistance;

		if (m_currentTime < task.getStart().ts)
		{
			m_currentTime = task.getStart().ts;
		}
	}

	m_currentNode = nextNode;
	pushStatsIfNeeded(nodeDistance);
}

bool CurrentRouteState::processLoad(const Task& task)
{
	//Process:
	m_currentTime += task.getStart().dur; // service time
	m_currentLoad += task.getCapacity();
	m_currentVolume += task.getVolume();
	++m_taskTaken;
	pushStatsIfNeeded(0);

	if (m_currentTime > task.getStart().te)
	{
		m_failsCollector.addEvent(MissedTimeFrameDuringLoad);
		return false;
	}//loosed time frame

	if (m_vehicle.getMaxVolume() != 0 && m_currentVolume > m_vehicle.getMaxVolume())
	{
		m_failsCollector.addEvent(ExceededMaxVolumeDuringLoad);
		return false;
	}

	if (m_vehicle.getCapacity() != 0 && m_currentLoad > m_vehicle.getCapacity())
	{
		m_failsCollector.addEvent(ExceededMaxCapacityDuringLoad);
		return false;
	}//capacity overlap

	if (m_vehicle.getTripMaxTasks() != 0 && m_taskTaken > m_vehicle.getTripMaxTasks())
	{
		m_failsCollector.addEvent(ExceededMaxTasksDuringLoad);
		return false;
	}

	return true;
}

bool CurrentRouteState::processUnload(const Task& task)
{
	if (m_taskTaken == 0)
	{
		m_failsCollector.addEvent(EmptyTasksDuringUnload);
		return false;
	}

	//Process:
	m_currentTime += task.getEnd().dur; // service time

	if (m_currentTime > task.getEnd().te)
	{
		m_failsCollector.addEvent(MissedTimeFrameDuringUnload);
		return false;
	}

	if (m_vehicle.getTripDuration() != 0 && (m_currentTime - m_routeStartTime)  > m_vehicle.getTripDuration())
	{
		m_failsCollector.addEvent(ExceededTripDuarationDuringUnload);
		return false;
	}

	if (m_vehicle.getTripLength() != 0 && m_currentDistanceMeters > m_vehicle.getTripLength())
	{
		m_failsCollector.addEvent(ExceededTripLengthDuringUnload);
		return false;
	}

	m_currentLoad -= task.getCapacity();
	m_currentVolume -= task.getVolume();
	--m_taskTaken;
	pushStatsIfNeeded(0);
	return true;
}

bool CurrentRouteState::moveNextEnd(int32_t nextNode, const Task& task)
{
	if (m_taskTaken == 0 || m_currentNode == -1)
	{
		m_failsCollector.addEvent(CantMoveToEndBeforeStart);
		return false;
	}

	m_currentTime += m_destTimeTable(m_currentNode, nextNode);
	double node_distance = m_destTable(m_currentNode, nextNode);
	m_currentDistanceMeters += node_distance;

	if (m_currentTime < task.getEnd().ts)
	{
		m_currentTime = task.getEnd().ts;
	}

	pushStatsIfNeeded(node_distance);
	return true;
}


std::size_t CurrentRouteState::totalTimeSpend() const
{
	return m_currentTime - m_routeStartTime;
}

double CurrentRouteState::totalDistance() const
{
	return m_currentDistanceMeters;
}

double CurrentRouteState::totalCostOfRoute() const
{
	double cost = 0;

	if (m_vehicle.costPerKM())
	{
		cost = m_currentDistanceMeters / 1000.0 * m_vehicle.getCostOfUsing();
	}
	else
	{
		cost = m_vehicle.getCostOfUsing() * totalTimeSpend() / 60.0 / 60.0;
	}

	return cost;
}

PointStat::PointStat(
	uint32_t nodeId,
	std::size_t time,
	double workload,
	double volume,
	double distance,
	int tasksCount
)
	: nodeId(nodeId)
	, time(time)
	, workload(workload)
	, volume(volume)
	, distance(distance)
	, tasksCount(tasksCount)
{
}



}
