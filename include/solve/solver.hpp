#pragma once

#include <vector>
#include <memory>
#include <string>

#include "parser/json_parser.hpp"

namespace rto {

enum ActionType
{
	Load,
	Unload
};

struct ResultTask
{
	std::uint32_t id = 0;
	ActionType action;
	std::size_t startTime = 0;
	std::size_t stopTime = 0;
	double workload = 0;
	double volume = 0;
	double distance = 0;
	int tasksCount = 0;
};

struct ResultRoute
{
	std::uint32_t vehicle = 0;
	std::size_t startTime = 0;
	std::size_t stopTime = 0;
	double distance = 0;
	std::string polyline;
	std::vector<ResultTask> tasks;
};

struct TaskError
{
	std::uint32_t taskId;
	std::string message;
};

struct TotalResult
{
	std::vector<TaskError> errors;
	std::vector<ResultRoute> routes;
};

class Solver {
public:
	~Solver();
	Solver(const std::string& osrm_data_path);

	TotalResult solve(const std::vector<rs::Vehicle>& vehicles,
					  const std::vector<rs::Zone>& workzones,
					  const std::vector<rs::Task>& tasks,
					  double speedForAproximationMetersInSecond,
					  bool partialSolutionEnabled,
					  bool multiThreaded,
					  const ABCParams& abcParams = ABCParams());
private:
	std::unique_ptr<class SolverPrivate> p;
};

}
