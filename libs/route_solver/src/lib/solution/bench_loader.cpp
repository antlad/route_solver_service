//
// Created by vlad on 26.11.16.
//

#include "route_solver/solution/bench_loader.hpp"

#include <fstream>
#include <sstream>

namespace rs {

class BenchCustomer {
public:
	size_t getNumber() const;
	size_t getX() const;
	size_t getY() const;
	size_t getDemand() const;
	size_t getReadyTime() const;
	size_t getDueDate() const;
	size_t getServiceTime() const;
private:
	BenchCustomer();
	std::size_t m_number;
	std::size_t m_x;
	std::size_t m_y;
	std::size_t m_demand;
	std::size_t m_readyTime;
	std::size_t m_dueDate;
	std::size_t m_serviceTime;
	friend class BenchLoader;
};

BenchLoader BenchLoader::initFromString(const std::string& caseData, int speed)
{
	std::stringstream ss(caseData);
	return BenchLoader(ss, speed);
}

BenchLoader::BenchLoader(
	std::istream& stream,
	std::size_t venchileSpeed)
	: m_venchileSpeed(venchileSpeed)
{
	std::vector<BenchCustomer> customers;
	stream >> m_name;
	std::string buffer;
	std::size_t venchilesNumber;
	std::size_t venchilesCapacity;

	for (int i = 0; i < 4; ++i)
	{
		std::getline(stream, buffer);
	}

	stream >> venchilesNumber >> venchilesCapacity;

	for (int i = 0; i < 4; ++i)
	{
		std::getline(stream, buffer);
	}

	while (stream.good())
	{
		BenchCustomer c;
		stream >> c.m_number >> c.m_x >> c.m_y >> c.m_demand >> c.m_readyTime >> c.m_dueDate >> c.m_serviceTime;
		customers.push_back(c);
	}

	for (std::size_t i = 0; i < venchilesNumber; ++i)
	{
		m_venchiles.emplace_back(i);
		auto& v = m_venchiles.back();
		v.setCapacity(venchilesCapacity);
		v.setCostOfUsing(1);
		v.setCostPerKM(true);
	}

	const auto& depot = customers[0];
	std::vector<point2d> points_for_calc;
	int taskId = 0;

	for (std::size_t i = 1; i < customers.size(); ++i)
	{
		const auto& c = customers[i];
		m_tasks.emplace_back(taskId++);
		auto& t = m_tasks.back();
		t.setCapacity(c.m_demand);
		t.setStart(TaskPoint(point2d(depot.getY(), depot.getX()), depot.getReadyTime(), depot.getDueDate(), depot.getServiceTime()));
		t.setEnd(TaskPoint(point2d(c.getY(), c.getX()), c.getReadyTime(), c.getDueDate(), c.getServiceTime()));
		points_for_calc.emplace_back(depot.getY(), depot.getX());
		points_for_calc.emplace_back(c.getY(), c.getX());
	}

	m_destTimeMatrix.resize(points_for_calc.size(), points_for_calc.size());
	m_destMatrix.resize(points_for_calc.size(), points_for_calc.size());

	for (std::size_t c = 0; c < points_for_calc.size(); ++c)
	{
		for (std::size_t r = 0; r < points_for_calc.size(); ++r)
		{
			double x1 = points_for_calc[c].x();
			double y1 = points_for_calc[c].y();
			double x2 = points_for_calc[r].x();
			double y2 = points_for_calc[r].y();
			double len = (double) sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
			m_destMatrix(r, c) = len;
			m_destTimeMatrix(r, c) = len / (double) m_venchileSpeed;
		}
	}
}

const std::string& BenchLoader::getName() const
{
	return m_name;
}
const std::vector<Vehicle>& BenchLoader::getVenchiles() const
{
	return m_venchiles;
}
const std::vector<Task>& BenchLoader::getTasks() const
{
	return m_tasks;
}
const Eigen::MatrixXd& BenchLoader::getDestTimeMatrix() const
{
	return m_destTimeMatrix;
}
const Eigen::MatrixXd& BenchLoader::getDestMatrix() const
{
	return m_destMatrix;
}

BenchCustomer::BenchCustomer()
{
}
size_t BenchCustomer::getNumber() const
{
	return m_number;
}
size_t BenchCustomer::getX() const
{
	return m_x;
}
size_t BenchCustomer::getY() const
{
	return m_y;
}
size_t BenchCustomer::getDemand() const
{
	return m_demand;
}
size_t BenchCustomer::getReadyTime() const
{
	return m_readyTime;
}
size_t BenchCustomer::getDueDate() const
{
	return m_dueDate;
}
size_t BenchCustomer::getServiceTime() const
{
	return m_serviceTime;
}
}
