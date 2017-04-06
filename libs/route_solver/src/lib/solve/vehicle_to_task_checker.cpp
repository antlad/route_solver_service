//
// Created by vlad on 14.11.16.
//

#include "route_solver/solve/vehicle_to_task_checker.hpp"

namespace rs {


VehicleTaskChecker::VehicleTaskChecker(
	const std::vector<Vehicle>& venchiles,
	const std::vector<Zone>& zones,
	const std::vector<Task>& tasks
)
{
	m_possibilityVehicleToTask.resize(venchiles.size(), tasks.size());
	m_possibilityVehicleToTask.fill(0);
	std::map<std::uint32_t, std::vector<std::uint32_t>> zoneAndTask;
	std::map<std::uint32_t, std::uint32_t> externalIdToLocal;

	for (const auto& z : zones)
	{
		externalIdToLocal[z.getExternalId()] = z.getId();

		for (const auto& t : tasks)
		{
			if (z.isTaskInZone(t))
			{
				zoneAndTask[z.getId()].push_back(t.getId());
			}
		}
	}

	for (const auto& v : venchiles)
	{
		if (v.getWorkZonesExternalIds().empty() && v.getWorkZonesExternalIds().empty())
		{
			for (const auto& t : tasks)
			{
				m_possibilityVehicleToTask(v.getId(), t.getId()) = 1;
			}
		}
		else if (!v.getWorkZonesExternalIds().empty())
		{
			for (const auto externalZoneId : v.getWorkZonesExternalIds())
			{
				auto localZoneId = externalIdToLocal[externalZoneId];

				for (auto taskId : zoneAndTask[localZoneId])
				{
					m_possibilityVehicleToTask(v.getId(), taskId) = 1;
				}
			}
		}
		else if (!v.getWorkZonesExternalIds().empty())
		{
			for (const auto& t : tasks)
			{
				m_possibilityVehicleToTask(v.getId(), t.getId()) = 1;
			}

			for (const auto externalZoneId : v.getWorkZonesExternalIds())
			{
				auto localZoneId = externalIdToLocal[externalZoneId];

				for (auto taskId : zoneAndTask[localZoneId])
				{
					m_possibilityVehicleToTask(v.getId(), taskId) = 0;
				}
			}
		}

		for (const auto& t : tasks)
		{
			if (!v.canTakeTask(t))
			{
				m_possibilityVehicleToTask(v.getId(), t.getId()) = 0;
			}
		}
	}
}

bool VehicleTaskChecker::taskAcceptableForVenchile(std::uint32_t taskId, std::uint32_t venhileId) const
{
	return m_possibilityVehicleToTask(venhileId, taskId) == 1;
}

const Eigen::MatrixXd& VehicleTaskChecker::possibilityVehicleToTask() const
{
	return m_possibilityVehicleToTask;
}

}
