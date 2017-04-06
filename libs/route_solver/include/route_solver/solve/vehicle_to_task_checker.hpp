//
// Created by vlad on 14.11.16.
//

#pragma once

#include "route_solver/structs/data_structs.hpp"

#include <Eigen/Core>

#include <map>
#include <vector>
#include <set>

namespace rs {

bool isPointInWorkZone(const point2d& pt, const std::vector<point2d>& workzone);

class VehicleTaskChecker {

public:
	VehicleTaskChecker(const std::vector <Vehicle>& venchiles,
					   const std::vector<Zone>& zones,
					   const std::vector <Task>& tasks);

	bool taskAcceptableForVenchile(std::uint32_t taskId, std::uint32_t venhileId) const;

	const Eigen::MatrixXd& possibilityVehicleToTask() const;

private:
	Eigen::MatrixXd m_possibilityVehicleToTask;

};

}
