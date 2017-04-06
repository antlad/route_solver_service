#include <route_solver/abc/abc.hpp>
#include <route_solver/log.hpp>
#include <route_solver/solution/bench_loader.hpp>
#include <route_solver/solution/truck_solution.hpp>
#include <route_solver/solve/vehicle_to_task_checker.hpp>
#include <route_solver/solve/truck_solver.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <map>

namespace {
static const int dump = []()
{
	rs::log::initializeJustConsole();
	return 0;
}();
}

namespace po = boost::program_options;
namespace bf = boost::filesystem;

int main(int argc, const char* argv[])
{
	try
	{
		std::string benchFile;
		std::size_t vehicleSpeed = 0;
		std::size_t colonySize = 0;
		std::size_t maxCycles = 0;
		std::size_t maxTrials = 0;
		std::size_t maxSeconds = 0;
		po::options_description desc("Allowed options");
		desc.add_options()
		("bench_file", po::value<std::string>(&benchFile)->required(), "input benchmark file")
		("speed", po::value<std::size_t>(&vehicleSpeed)->default_value(10), "vehicle speed")
		("colony_size", po::value<std::size_t>(&colonySize)->default_value(100), "colony size")
		("max_cycles", po::value<std::size_t>(&maxCycles)->default_value(2000000), "max cycles iterations")
		("max_trials", po::value<std::size_t>(&maxTrials)->default_value(10), "max trials per solution")
		("max_seconds", po::value<std::size_t>(&maxSeconds)->default_value(10), "max seconds to work")
		("help", "produce help message");
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
		auto check_path = [](const auto & path)
		{
			if (!bf::exists(path))
			{
				const auto& s = (boost::format("File path %s doesn't exists") % path).str();
				BOOST_THROW_EXCEPTION(std::runtime_error(s));
			}
		};
		check_path(benchFile);
		std::ifstream fs;
		fs.open(benchFile);
		rs::BenchLoader caseData(fs, vehicleSpeed);
		auto solution = rs::truckSolve(
							caseData.getVenchiles(),
							std::vector<rs::Zone>(),
							caseData.getTasks(),
							caseData.getDestTimeMatrix(),
							caseData.getDestMatrix(),
							rs::BeeColonyParams(colonySize, maxCycles, maxTrials, maxSeconds),
							false
						);
		LOG_TRACE << "solved, fitness " << solution.fitness();
		LOG_TRACE << "total routes count " << solution.getRoutes().size();
		LOG_TRACE << "total distance " << solution.getStat().totalDistance;
		LOG_TRACE << "total time " << solution.getStat().totalTime;

		for (const auto& r : solution.getRoutes())
		{
			std::stringstream ss;

			for (const auto& n : r.nodes)
			{
				int m = n % 2;

				if (m == 0) //because every task start is depot
				{
					ss  << 0 << " ";
				}
				else
				{
					ss  << n / 2 << " ";
				}
			}

			LOG_TRACE << "id " << r.vehicleId << " : " << ss.str();
		}

		if (vm.count("help") || argc < 2)
		{
			std::cout << desc << std::endl;
			return 0;
		}
	}
	catch (const boost::exception& e)
	{
		std::cerr << boost::diagnostic_information(e);
		return 1;
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << e.what();
		return 1;
	}

	return 0;
}
