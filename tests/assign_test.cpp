//
// Created by vlad on 14.11.16.
//

#include <gtest/gtest.h>
#include <route_solver/solve/vehicle_to_task_checker.hpp>
#include <parser/json_parser.hpp>

#include "test_data.hpp"

using namespace rto;
using namespace rs;

template<class T>
const T* objByExternalId(const std::vector<T>& obj, std::uint32_t externald)
{
	for (const auto& z : obj)
	{
		if (z.getExternalId() == externald)
		{
			return &z;
		}
	}

	return 0;
}

TEST(AssignTest, CheckerSimpleTest)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	std::vector<Task> testTasks;
	testTasks.emplace_back(taskFromJson(0, R"({
	  "id": 0,
	  "capacity": 0,
	  "volume": 0,
	  "maxsize": 0,
	  "vip": 0,
	  "types": [],
	  "start": {
		"lat": 55.739772,
		"lon": 37.518826,
		"ts": 1477458000,
		"te": 1477461600,
		"dur": 120
	  },
	  "end": {
		"lat": 55.739778,
		"lon": 37.518828,
		"ts": 1477494000,
		"te": 1477497600,
		"dur": 900
	  }
})"_json));
	testTasks.emplace_back(taskFromJson(1, R"({
	  "id": 1,
	  "capacity": 0,
	  "volume": 0,
	  "maxsize": 0,
	  "vip": 0,
	  "types": [1,2],
	  "start": {
		"lat": 55.735713,
		"lon": 37.464752,
		"ts": 1477458000,
		"te": 1477461600,
		"dur": 120
	  },
	  "end": {
		"lat": 55.735715,
		"lon": 37.464754,
		"ts": 1477494000,
		"te": 1477497600,
		"dur": 900
	  }
})"_json));
	VehicleTaskChecker check(p.getVenchiles(), p.getZones(), testTasks);
	auto id = objByExternalId<Vehicle>(p.getVenchiles(), 12288)->getId();
	EXPECT_TRUE(check.taskAcceptableForVenchile(0, id));
	EXPECT_FALSE(check.taskAcceptableForVenchile(1, id));
}

TEST(ByWorkZoneTest, Task_66191)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	VehicleTaskChecker check(p.getVenchiles(), p.getZones(), p.getTasks());
	//w 6272 6529
	auto t = objByExternalId<Task>(p.getTasks(), 66191);
	auto v0 = objByExternalId<Vehicle>(p.getVenchiles(), 12288);
	auto v1 = objByExternalId<Vehicle>(p.getVenchiles(), 12286);
	auto v2 = objByExternalId<Vehicle>(p.getVenchiles(), 12284);
	std::cout << t->getId() << " " << v0->getId() << " " << v1->getId() << " " << v2->getId() << std::endl;
	std::cout << check.possibilityVehicleToTask() << std::endl;
	EXPECT_TRUE(check.taskAcceptableForVenchile(t->getId(), v0->getId()));
	EXPECT_TRUE(check.taskAcceptableForVenchile(t->getId(), v1->getId()));
	EXPECT_TRUE(check.taskAcceptableForVenchile(t->getId(), v2->getId()));
}


TEST(TypesTest, Vehicle_12286)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	auto v = objByExternalId<Vehicle>(p.getVenchiles(), 12288);
	VehicleTaskChecker check(p.getVenchiles(), p.getZones(), p.getTasks());
	auto task = [&](auto id)
	{
		return objByExternalId<Task>(p.getTasks(), id)->getId();
	};
	//std::cout << check.possibilityVehicleToTask() << std::endl;
	EXPECT_TRUE(check.taskAcceptableForVenchile(task(66151), v->getId()));
	//EXPECT_TRUE(check.taskAcceptableForVenchile(task(66151), v->getId()));
}
