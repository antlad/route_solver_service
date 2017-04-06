#pragma once

#include <cstddef>

namespace rs {

template<class T, class S>
class SolutionProducer {
public:
	S generateRandom()
	{
		return static_cast<T*>(this)->generateRandomImpl();
	}

	S produceNeighborhood(const S& solution)
	{
		return static_cast<T*>(this)->produceNeighborhoodImpl(solution);
	}

	void mergeFails()
	{
		return static_cast<T*>(this)->mergeFails();
	}

};

template<class T>
class Solution {
public:
	std::size_t getTrialCounter() const
	{
		return m_trialCounter;
	}

	std::size_t increaseTrialCounter()
	{
		return ++m_trialCounter;
	}

	double fitness() const
	{
		return static_cast<const T*>(this)->fitnessImpl();
	}

private:
	std::size_t m_trialCounter = 0;
};

struct Statistics
{
	std::size_t totalCycles = 0;
	std::size_t totalMicroSeconds = 0;
};

struct BeeColonyParams
{
	BeeColonyParams(
		std::size_t colonySize,
		std::size_t trialCountMax,
		std::size_t maxTimeInSecHardLimit,
		std::size_t maxTimeInSecNoNewBestSolution
	)
		: colonySize(colonySize)
		, trialCountMax(trialCountMax)
		, maxTimeInSecHardLimit(maxTimeInSecHardLimit)
		, maxTimeInSecNoNewBestSolution(maxTimeInSecNoNewBestSolution)
	{
	}

	std::size_t colonySize;// the number of employed or onlooker bees
	std::size_t trialCountMax;
	std::size_t maxTimeInSecHardLimit;
	std::size_t maxTimeInSecNoNewBestSolution;
};

}
