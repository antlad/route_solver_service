#include "route_solver/solution/fail_statistics.hpp"

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace rs {
FailedStatisticsCollector::FailedStatisticsCollector()
	: m_events(MaxElement, 0)
{
}

void FailedStatisticsCollector::addEvent(FailEvent e)
{
	m_events[e] += 1;
}

FailedStatisticsCollector& FailedStatisticsCollector::operator +=(const FailedStatisticsCollector& other)
{
	if (other.m_events.size() != m_events.size())
	{
		BOOST_THROW_EXCEPTION(std::runtime_error("Unexpected events size!"));
	}

	auto it1 = other.m_events.begin();
	auto it2 = m_events.begin();

	while (it2 != m_events.end())
	{
		*it2++ = *it1++;
	}

	return *this;
}

std::string FailedStatisticsCollector::print() const
{
	std::stringstream ss;
	ss << "\nFailes stats\n";
	typedef std::pair<std::size_t, std::size_t> FailStat;
	std::vector<FailStat> sorted_events;

	for (std::size_t i = 0; i < m_events.size(); ++i)
	{
		sorted_events.emplace_back(i, m_events[i]);
	}

	std::sort(sorted_events.begin(), sorted_events.end(), [](const FailStat & a, const FailStat & b)
	{
		return a.second > b.second;
	});

	for (const auto& e : sorted_events)
	{
		FailEvent fe = (FailEvent)e.first;
		ss << FailEventToString(fe) << ": " << e.second << "\n";
	}

	return ss.str();
}
}
