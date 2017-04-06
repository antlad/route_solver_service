//
// Created by vlad on 03.12.16.
//

#pragma once

#include <cstdlib>
#include <chrono>

namespace rs {
namespace utils {

//std::size_t unix_now();

// TODO: implement somthing like https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor/

template<class T>
T randomNumber(T min, T max)
{
	double scaled = (double) std::rand() / RAND_MAX;
	return (max - min + 1) * scaled + min;
}


class TimerElapsed {
public:
	TimerElapsed();
	std::size_t elapsedMiliseconds() const;
private:
	std::chrono::steady_clock::time_point m_start;
};
}
}
