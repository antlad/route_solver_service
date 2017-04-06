
#pragma once

#include "route_solver/log.hpp"
#include "route_solver/abc/abc_common.hpp"

#include <boost/atomic.hpp>

#include <vector>
#include <chrono>
#include <algorithm>
#include <thread>

namespace rs {

class SpinLock {
private:
	typedef enum {Locked, Unlocked} LockState;
	boost::atomic<LockState> m_state;

public:
	SpinLock() : m_state(Unlocked) {}

	void lock()
	{
		while (m_state.exchange(Locked, boost::memory_order_acquire) == Locked)
		{
			/* busy-wait */
		}
	}
	void unlock()
	{
		m_state.store(Unlocked, boost::memory_order_release);
	}
};

class WorkerState {
private:
	typedef enum {RunningState, StopState} State;
	boost::atomic<State> m_state;
public:
	WorkerState(): m_state(RunningState) {}
	void doStop()
	{
		m_state.store(StopState, boost::memory_order_acquire);
	}

	bool isActive() const
	{
		return m_state.load(boost::memory_order_relaxed) == RunningState;
	}
};

template<class T, class S>
void colonyRoutine(const std::function<T()>& helperFabric,
				   const BeeColonyParams& params,
				   const WorkerState& flag,
				   SpinLock& sync,
				   double& globalBestsolutionFitness,
				   S& bestSolutionOutput
				  )
{
	auto helper = helperFabric();
	std::size_t totalSolCount = params.colonySize / 2;
	std::vector<S> solutions;
	std::vector<std::pair<double, std::size_t>> solutionsProbability(totalSolCount);
	std::vector<std::size_t> onSeekersIndexes(totalSolCount);
	std::size_t maxSeekersCount = totalSolCount;
	auto bestSolution = helper.generateRandom();

	for (std::size_t i = 0; i < totalSolCount; ++i)
	{
		solutions.emplace_back(helper.generateRandom());
	}

	while (flag.isActive())
	{
		for (std::size_t s = 0; s < totalSolCount; ++s)
		{
			auto& oldSol = solutions[s];
			auto newSol = helper.produceNeighborhood(oldSol);

			if (newSol.fitness() > oldSol.fitness())
			{
				solutions[s] = std::move(newSol);
			}
			else
			{
				oldSol.increaseTrialCounter();
			}
		}

		double fitnessSum = 0;

		for (const auto& f : solutions)
		{
			fitnessSum += f.fitness();
		}

		for (std::size_t s = 0; s < solutionsProbability.size(); ++s)
		{
			solutionsProbability[s] = std::make_pair(solutions[s].fitness() / fitnessSum, s);
		}

		std::sort(solutionsProbability.begin(),
				  solutionsProbability.end(), [](const auto & a, const auto & b)
		{
			return a.first > b.first;
		});
		std::size_t activeOnSeekers = 0;

		for (const auto& p : solutionsProbability)
		{
			std::size_t count = p.first * totalSolCount + 1;

			for (; count > 0 && activeOnSeekers < maxSeekersCount; ++activeOnSeekers, --count)
			{
				onSeekersIndexes[activeOnSeekers] = p.second;
			}
		}

		for (std::size_t s = 0; s < activeOnSeekers; ++s)
		{
			auto index = onSeekersIndexes[s];
			auto& oldSol = solutions[index];
			auto newSol = helper.produceNeighborhood(oldSol);;

			if (newSol.fitness() > oldSol.fitness())
			{
				solutions[s] = newSol;
			}
			else
			{
				if (oldSol.increaseTrialCounter() > params.trialCountMax)
				{
					oldSol = helper.generateRandom();
				}
			}
		}

		for (const auto& s : solutions)
		{
			if (s.fitness() > bestSolution.fitness())
			{
				bestSolution = s;
			}
		}

		double bestSolutionFitness = bestSolution.fitness();
		bool our_is_best = false;
		sync.lock();

		if (bestSolutionFitness > globalBestsolutionFitness)
		{
			globalBestsolutionFitness = bestSolutionFitness;
			our_is_best = true;
		}

		sync.unlock();

		if (!our_is_best)
		{
			for (auto& s : solutions)
			{
				s = helper.generateRandom();
			}
		}
	}

	sync.lock();

	if (bestSolution.fitness() > bestSolutionOutput.fitness())
	{
		bestSolutionOutput = bestSolution;
	}

	helper.mergeFails();
	sync.unlock();
}


template<class T, class S>
auto beeColonySolveParallel(const std::function<T()>& helperFabric,
							const BeeColonyParams& params,
							Statistics* outStatistics = 0)
{
	auto helper = helperFabric();
	auto bestSolution = helper.generateRandom();
	SpinLock sync;
	WorkerState flag;
	double globalBestSolutionFitness = bestSolution.fitness();
	double syncGlobalBestSolutionFitness = bestSolution.fitness();
	std::vector<std::thread> threads;

	if (outStatistics)
	{
		outStatistics->totalCycles = 0;
		outStatistics->totalMicroSeconds = 0;
	}

	std::size_t threads_num = std::thread::hardware_concurrency();

	for (std::size_t i = 0; i < threads_num; ++i)
	{
		threads.emplace_back(
			colonyRoutine<T, S>,
			std::ref(helperFabric),
			std::ref(params),
			std::ref(flag),
			std::ref(sync),
			std::ref(syncGlobalBestSolutionFitness),
			std::ref(bestSolution)
		);
	}

	auto start = std::chrono::steady_clock::now();
	auto lastUpdateTime = std::chrono::steady_clock::now();

	for (std::size_t i = 0;; ++i)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		bool new_best_found = false;
		sync.lock();

		if (syncGlobalBestSolutionFitness > globalBestSolutionFitness)
		{
			new_best_found = true;
			globalBestSolutionFitness = syncGlobalBestSolutionFitness;
		}

		sync.unlock();

		if (new_best_found)
		{
			lastUpdateTime = std::chrono::steady_clock::now();
			LOG_DEBUG << "in parallel, new best solution found, fitness " << std::to_string(globalBestSolutionFitness);
		}

		std::chrono::steady_clock::time_point current = std::chrono::steady_clock::now();
		auto current_seconds = (size_t) std::chrono::duration_cast<std::chrono::seconds>(current - start).count();
		auto last_update_seconds = (size_t) std::chrono::duration_cast<std::chrono::seconds>(current - lastUpdateTime).count();

		if (current_seconds > params.maxTimeInSecHardLimit || last_update_seconds > params.maxTimeInSecNoNewBestSolution)
		{
			if (outStatistics)
			{
				std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
				outStatistics->totalMicroSeconds = (size_t) std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
				outStatistics->totalCycles = i;
			}

			break;
		}
	}

	flag.doStop();

	for (auto& t : threads)
	{
		t.join();
	}

	return bestSolution;
}
}
