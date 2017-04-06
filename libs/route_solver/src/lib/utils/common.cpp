//
// Created by vlad on 03.12.16.
//
#include <route_solver/utils/common.hpp>

namespace rs {
namespace utils {

TimerElapsed::TimerElapsed()
	: m_start(std::chrono::steady_clock::now())
{
}

std::size_t TimerElapsed::elapsedMiliseconds() const
{
	std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
	return (size_t) std::chrono::duration_cast<std::chrono::milliseconds>(current - m_start).count();
}


}
}
