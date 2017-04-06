#pragma once

#include <route_solver/structs/data_structs.hpp>
#include <json.hpp>

namespace rto {

rs::Zone zoneFromJson(uint32_t id, const nlohmann::json& j);
rs::Vehicle vehicleFromJson(std::uint32_t id, const nlohmann::json& j);
rs::Task taskFromJson(std::uint32_t id, const nlohmann::json& j);

struct ABCParams
{
	std::size_t colonySize = 100;
	std::size_t trialCountMax = 50;
	std::size_t maxTimeInSecHardLimit = 60 * 5;
	std::size_t maxTimeInSecNoNewBestSolution = 30;

};

class JsonParser {
public:

	static JsonParser fromString(const std::string& raw);

	JsonParser(const nlohmann::json& root);

	const std::vector<rs::Vehicle>& getVenchiles() const;

	const std::vector<rs::Zone>& getZones() const;

	const std::vector<rs::Task>& getTasks() const;

	void produceInputDataCsvToFolder(const std::string& folder) const;

	double getSpeedAprox() const;

	std::string getLogLevel() const;

	void setTasks(const std::vector<rs::Task>& tasks);

	const ABCParams& getAbcParams() const;

	std::uint32_t getAccountId() const;

	bool getPartialSolutionEnabled() const;

	bool getMultiThreaded() const;

private:
	std::vector<rs::Zone> m_zones;
	std::vector<rs::Vehicle> m_venchiles;
	std::vector<rs::Task> m_tasks;
	std::uint32_t m_accountId = 0;
	ABCParams m_abcParams;
	std::string m_logLevel;
	double m_speedAprox = 90;
	bool m_multiThreaded = false;
	bool m_partialSolutionEnabled = false;
};
}
