#include <route_solver/utils/common.hpp>

#include <gtest/gtest.h>

using namespace rs;
TEST(RandomTest, case1)
{
	bool zeroFound = false;
	bool lastFound = false;

	for (int i = 0; i < 100000; ++i)
	{
		auto num = utils::randomNumber(0, 10);

		if (num == 0)
		{
			zeroFound = true;
		}
		else if (num == 10)
		{
			lastFound = true;
		}
		else if (num < 0 || num > 10)
		{
			FAIL();
		}
	}

	EXPECT_EQ(true, zeroFound);
	EXPECT_EQ(true, lastFound);
}


