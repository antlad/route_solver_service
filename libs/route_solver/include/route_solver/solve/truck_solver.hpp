#pragma once

#include <route_solver/structs/data_structs.hpp>
#include <route_solver/solution/truck_solution.hpp>

#include <Eigen/Core>

namespace rs {
rs::TruckSolution truckSolve(
	const std::vector<Vehicle>& venchiles,
	const std::vector<rs::Zone>& zones,
	const std::vector<Task>& tasks,
	const Eigen::MatrixXd& destTimeMatrix,
	const Eigen::MatrixXd& getDestMatrix,
	const BeeColonyParams& beeParams,
	bool partialSolutionEnabled,
	rs::Statistics* stat = 0
);

}
