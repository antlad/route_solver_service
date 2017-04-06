//
// Created by vlad on 14.11.16.
//

#include <gtest/gtest.h>
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

TEST(WorkzoneTest, WorkZoneAlgorithmTest_6272)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	const auto* z = objByExternalId<Zone>(p.getZones(), 6272);
	EXPECT_NE(nullptr, z);
	EXPECT_TRUE(isPointInZone(point2d(55.739772, 37.518826), z->getPoints()));
	EXPECT_TRUE(isPointInZone(point2d(55.740545, 37.544060), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.735713, 37.464752), z->getPoints()));
}

TEST(WorkzoneTest, WorkZoneAlgorithmTest_6273)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	const auto* z = objByExternalId<Zone>(p.getZones(), 6273);
	EXPECT_NE(nullptr, z);
	EXPECT_TRUE(isPointInZone(point2d(55.695777, 37.440720), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.671970, 37.552471), z->getPoints()));
}

TEST(WorkzoneTest, WorkZoneAlgorithmTest_7748)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	const auto* z = objByExternalId<Zone>(p.getZones(), 7748);
	EXPECT_NE(nullptr, z);
	EXPECT_TRUE(isPointInZone(point2d(55.723000, 37.612800), z->getPoints()));
	EXPECT_TRUE(isPointInZone(point2d(55.733400, 37.520500), z->getPoints()));
}

TEST(WorkzoneTest, WorkZoneAlgorithmTest_6452)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	const auto* z = objByExternalId<Zone>(p.getZones(), 6452);
	EXPECT_NE(nullptr, z);
	EXPECT_TRUE(isPointInZone(point2d(55.719763, 37.423210), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.730010, 37.337379), z->getPoints()));
}

TEST(WorkzoneTest, WorkZoneAlgorithmTest_6472)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	const auto* z = objByExternalId<Zone>(p.getZones(), 6472);
	EXPECT_NE(nullptr, z);
	EXPECT_TRUE(isPointInZone(point2d(55.732233, 37.580023), z->getPoints()));
	EXPECT_TRUE(isPointInZone(point2d(55.731943, 37.575130), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.754941, 37.584486), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.747888, 37.539854), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.798483, 37.474022), z->getPoints()));
}

TEST(WorkzoneTest, WorkZoneAlgorithmTest_6473)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	const auto* z = objByExternalId<Zone>(p.getZones(), 6473);
	EXPECT_NE(nullptr, z);
	EXPECT_TRUE(isPointInZone(point2d(55.752139, 37.589808), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.761026, 37.565432), z->getPoints()));
}

TEST(WorkzoneTest, WorkZoneAlgorithmTest_6525)
{
	auto p = JsonParser::fromString(unit_tests_data::task1);
	const auto* z = objByExternalId<Zone>(p.getZones(), 6525);
	EXPECT_NE(nullptr, z);
	EXPECT_TRUE(isPointInZone(point2d(55.850939, 37.692547), z->getPoints()));
	EXPECT_FALSE(isPointInZone(point2d(55.890523, 37.664051), z->getPoints()));
}
