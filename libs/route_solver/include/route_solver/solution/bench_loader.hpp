//
// Created by vlad on 26.11.16.
//

#pragma once

#include "route_solver/structs/data_structs.hpp"

#include <Eigen/Core>

#include <string>
#include <vector>

namespace rs {
class BenchLoader {
public:
	static BenchLoader initFromString(const std::string& caseData, int speed);
	BenchLoader(std::istream& stream, std::size_t venchileSpeed);
	const std::vector<Vehicle>& getVenchiles() const;
	const std::string& getName() const;
	const std::vector<Task>& getTasks() const;
	const Eigen::MatrixXd& getDestTimeMatrix() const;
	const Eigen::MatrixXd& getDestMatrix() const;
private:
	std::string m_name;
	std::vector<Vehicle> m_venchiles;
	std::vector<Task> m_tasks;
	std::size_t m_venchileSpeed;
	Eigen::MatrixXd m_destTimeMatrix;
	Eigen::MatrixXd m_destMatrix;
};
}
