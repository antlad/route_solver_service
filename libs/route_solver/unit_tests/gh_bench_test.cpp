//
// Created by vlad on 26.11.16.
//
#include <route_solver/abc/abc.hpp>
#include <route_solver/log.hpp>

#include <route_solver/solution/bench_loader.hpp>
#include <route_solver/solve/vehicle_to_task_checker.hpp>
#include <route_solver/solution/truck_solution.hpp>
#include <route_solver/solve/truck_solver.hpp>
#include <gtest/gtest.h>

namespace {
static const int dump = []()
{
	rs::log::initializeJustConsole();
	return 0;
}();
}

static const std::string dummy_50 = R"(r1_2_4

VEHICLE
NUMBER     CAPACITY
  50          200

CUSTOMER
CUST NO.  XCOORD.    YCOORD.    DEMAND   READY TIME  DUE DATE   SERVICE TIME

0      70         70          0          0        634          0
1     107         77         34          0        587         10
2     109        139          8          0        545         10
3     120         22         39        124        134         10
4      48         47         19          0        593         10
5     116         22         32          0        558         10
6      12        138         14        285        295         10
7      86         40         22          0        590         10
8     121        124         21          0        550         10
9      61         57         35          0        609         10
10      40        113         23          0        572         10
11     129         24         18          0        550         10
12      12         84         16          0        565         10
13      44        116         27          0        572         10
14     102         52         15          0        588         10
15      41         36         13         61         71         10
16     132        133         24          0        536         10
17     104        139         29          0        548         10
18     104         54         13          0        587         10
19      22        104         11          0        566         10
20      46        133          7          0        557         10
21     138         78         43          0        556         10
22      16         92         10          0        566         10
23      18        104         18        347        357         10
24      66         82         27          0        612         10
25     107         25         13          0        566         10
26     139         73         10          0        555         10
27     101          0          9          0        548         10
28      90         14         31          0        565         10
29      20         69         13        388        398         10
30      64        132          8         67         77         10
31     115         82         30          0        578         10
32      54        106          9          0        585         10
33      30         21         12          0        561         10
34      63        129          7        216        226         10
35      82        100         18          0        592         10
36     108         30         18          0        569         10
37      37         73         33          0        591         10
38      50        112         18          0        578         10
39      47         83          1          0        598         10
40      92        138         12        412        422         10
41      81         36         17          0        589         10
42     115        124         11        421        431         10
43      13         48         15          0        563         10
44     113         35          9          0        569         10
45      60         84         23          0        607         10
46      44         58         30          0        596         10
47       8         87         17          0        560         10
48     116        105         23          0        567         10
49     117          4          9          0        543         10
50     140         42         13          0        549         10
)";

TEST(DummyTest, case0)
{
	LOG_DEBUG << "start working";

	static const auto caseData = rs::BenchLoader::initFromString(dummy_50, 100);

	rs::Statistics stat;

	auto solution = rs::truckSolve(
				caseData.getVenchiles(),
				std::vector<rs::Zone>(),
				caseData.getTasks(),
				caseData.getDestTimeMatrix(),
				caseData.getDestMatrix(),
				rs::BeeColonyParams(100, 2000000, 10, 10),
				&stat
				);

	LOG_DEBUG << "total cycles " << stat.totalCycles;
	LOG_DEBUG << "total time seconds " << (double)stat.totalMicroSeconds / 100000.0;
	// before optimiation:
	// total cycles 34160
	// total time seconds 110.003

	if (solution.fitness() > 0)
	{
		LOG_TRACE << "solved, fitness " << solution.fitness();

		for (const auto& r : solution.getRoutes())
		{
			std::stringstream ss;

			for (const auto& n : r.nodes)
			{
				ss  << n << " ";
			}

			LOG_TRACE << "id " << r.vehicleId << " : " << ss.str();
		}
	}
	else
	{
		LOG_TRACE << "not solved";
	}
}


#ifdef CUDA_ENABLED


#endif
