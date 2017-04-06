//
// Created by vlad on 02.12.16.
//

#include <route_solver/abc/abc.hpp>
#include <route_solver/utils/common.hpp>

#include <gtest/gtest.h>

#ifdef CUDA_ENABLED
#include <Eigen/LU>
#include <Eigen/Core>
#include <Eigen/Dense>
#include "cuda_func_opt.hpp"
#endif

namespace rs {
class FuncSolution
	: public Solution<FuncSolution> {
public:

	FuncSolution(double x1, double x2)
		: x1(x1), x2(x2), value(x1 * x1 + x2 * x2)
	{}

	FuncSolution(const FuncSolution& other) = default;

	double fitnessImpl() const
	{
		if (value >= 0)
		{
			return 1 / (1 + value);
		}
		else
		{
			return 1 + fabs(value);
		}
	}

	double getX1() const
	{
		return x1;
	}
	double getX2() const
	{
		return x2;
	}
	double getValue() const
	{
		return value;
	}

private:

	double x1;
	double x2;
	double value;
};

class FuncSolutionProducer
	: public SolutionProducer<FuncSolutionProducer, FuncSolution> {
public:

	void newBetterSolutionFoundImpl(const FuncSolution&) const
	{
	}
	FuncSolution generateRandomImpl() const
	{
		return FuncSolution(newValue(), newValue());
	}

	FuncSolution produceNeighborhoodImpl(const FuncSolution& solution) const
	{
		return FuncSolution(solution.getX1() + utils::randomNumber(-1.0, 1.0),
							solution.getX2() + utils::randomNumber(-1.0, 1.0));
	}

private:
	static double newValue()
	{
		return utils::randomNumber(-5.0, 5.0);
	}
};

}

TEST(ABCTests, simple_func_optimization)
{
	std::srand(unsigned(std::time(0)));
	auto s = rs::beeColonySolve<rs::FuncSolutionProducer, rs::FuncSolution>([]()
	{
		return rs::FuncSolutionProducer();
	}
	, rs::BeeColonyParams(6, 1000, 20, 5));
	std::cout << " x1 " << s.getX1() << " x2 " << s.getX2() << " value " << s.getValue() << std::endl;
}

#ifdef CUDA_ENABLED

static int a = ei_test_init_cuda();

double dot_cpu(const std::vector<Eigen::Vector3d>& v1, const std::vector<Eigen::Vector3d>& v2)
{
	double x = 0;

	for (int i = 0; i < v1.size(); ++i)
	{
		x += v1[i].dot(v2[i]);
	}

	return x;
}


static const int N = 10000000;

TEST(MatrixOps, case_cuda)
{
	std::vector<Eigen::Vector3d> v1(N, Eigen::Vector3d{ 1.0, 1.0, 1.0 });
	std::vector<Eigen::Vector3d> v2(N, Eigen::Vector3d{ -1.0, 1.0, 1.0 });
	double x = dot_cuda(v1, v2);
}

TEST(MatrixOps, case_cpu)
{
	std::vector<Eigen::Vector3d> v1(N, Eigen::Vector3d{ 1.0, 1.0, 1.0 });
	std::vector<Eigen::Vector3d> v2(N, Eigen::Vector3d{ -1.0, 1.0, 1.0 });
	double x = dot_cpu(v1, v2);
}

#endif
