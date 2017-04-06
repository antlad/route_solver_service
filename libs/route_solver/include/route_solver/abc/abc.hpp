//
// Created by vlad on 29.11.16.
//

#pragma once

#include "route_solver/log.hpp"
#include "route_solver/abc/abc_common.hpp"

#include <vector>
#include <chrono>
#include <algorithm>

namespace rs {


template<class T, class S>
auto beeColonySolve(const std::function<T()>& helperFabric,
					const BeeColonyParams& params,
					Statistics* outStatistics = 0)
{
	std::size_t totalSolCount = params.colonySize / 2;
	std::vector<S> solutions;
	auto helper = helperFabric();

	for (std::size_t i = 0; i < totalSolCount; ++i)
	{
		solutions.emplace_back(helper.generateRandom());
	}

	std::vector<std::pair<double, std::size_t>> solutionsProbability(totalSolCount);
	std::vector<std::size_t> onSeekersIndexes(totalSolCount);
	std::size_t maxSeekersCount = totalSolCount;
	auto bestSolution = helper.generateRandom();
	auto start = std::chrono::steady_clock::now();
	auto lastUpdateTime = std::chrono::steady_clock::now();

	if (outStatistics)
	{
		outStatistics->totalCycles = 0;
		outStatistics->totalMicroSeconds = 0;
	}

	for (std::size_t i = 0;; ++i)
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
				LOG_DEBUG << "new best solution found, fitness " << std::to_string(s.fitness());
				bestSolution = s;
				lastUpdateTime = std::chrono::steady_clock::now();
			}
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

	helper.mergeFails();
	return bestSolution;
}



}
