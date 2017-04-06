#include <route_solver/solve/truck_solver.hpp>
#include <route_solver/solve/vehicle_to_task_checker.hpp>
#include <route_solver/solution/truck_solution.hpp>
#include <route_solver/abc/abc.hpp>
#include <route_solver/abc/abc_parallel.hpp>

namespace rs {

rs::TruckSolution truckSolve(
	const std::vector<Vehicle>& venchiles,
	const std::vector<Zone>& zones,
	const std::vector<Task>& tasks,
	const Eigen::MatrixXd& destTimeMatrix,
	const Eigen::MatrixXd& getDestMatrix,
	const BeeColonyParams& beeParams,
	bool partialSolutionEnabled,
	rs::Statistics* stat
)
{
	rs::VehicleTaskChecker checker(venchiles, zones, tasks);
	return rs::beeColonySolveParallel<rs::TruckSolutionProducer, rs::TruckSolution>(
			   [&]()
	{
		return rs::TruckSolutionProducer(
				   destTimeMatrix,
				   getDestMatrix,
				   checker.possibilityVehicleToTask(),
				   venchiles,
				   tasks,
				   partialSolutionEnabled
			   );
	}
	, beeParams, stat);
	LOG_TRACE << rs::TruckSolutionProducer::getFailsCollector().print();
}

}
