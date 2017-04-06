#include "parser/json_parser.hpp"
#include "route_solver/log.hpp"

#include <boost/throw_exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>

namespace rto {

void setZoneCircle(const nlohmann::json& j, rs::Zone& zone)
{
	std::vector<double> vals;

	for (const auto& p : j["points"])
	{
		vals.push_back(p);
	}

	// TODO: check if coefficient is correct
	zone.setCircle(vals[2] / 111.32, rs::point2d(vals[0], vals[1]));
}

void setZonePolygonPoints(const nlohmann::json& j, rs::Zone& zone)
{
	std::vector<rs::point2d> points;

	for (const auto& f : j["points"])
	{
		float lat = 0;
		float lon = 0;
		int i = 0;

		for (const auto& a : f)
		{
			switch (i++)
			{
			case 0:
				lat = a;
				break;

			case 1:
				lon = a;
				break;

			default:
				BOOST_THROW_EXCEPTION(std::runtime_error("unexpected zone type"));
			}
		}

		points.push_back(rs::point2d(lat, lon));
	}

	if (points.size() > 1)
	{
		points.push_back(points.front());
	}

	zone.setPolygonPoints(std::move(points));
}


rs::Zone zoneFromJson(uint32_t id, const nlohmann::json& j)
{
	rs::Zone z(id);
	z.setExternalId(j["id"]);
	int zone_type_int = j["zonetype"];

	switch (zone_type_int)
	{
	case 1:
		setZonePolygonPoints(j, z);
		break;

	case 2:
		setZoneCircle(j, z);
		break;

	default:
		BOOST_THROW_EXCEPTION(std::runtime_error("Unsupported zone type"));
	}

	return z;
}

rs::TaskPoint taskPointFromJson(const nlohmann::json& j)
{
	return rs::TaskPoint(rs::point2d(j["lat"], j["lon"]), j["ts"], j["te"], j["dur"]);
}

template<class T>
T get_from_string(const std::string& name, const nlohmann::json& j)
{
	return boost::lexical_cast<T>(j[name].get<std::string>());
}

rs::Vehicle vehicleFromJson(std::uint32_t id, const nlohmann::json& j)
{
	rs::Vehicle v(id);
	v.setExternalId(j["id"]);
	v.setCapacity(get_from_string<float>("capacity", j));
	v.setLength(get_from_string<float>("length", j));
	v.setHeight(get_from_string<float>("height", j));
	v.setWidth(get_from_string<float>("width", j));
	v.setTripDuration(j["trip_duration"].get<double>());
	v.setTripLength(j["trip_length"].get<double>());
	v.setCostOfUsing(get_from_string<float>("cost_of_using", j));
	v.setCostPerKM(j["cost_per_km"].get<bool>());
	v.setTripMaxTasks(get_from_string<std::uint32_t>("trip_max_tasks", j));
	std::vector<std::uint32_t> wzones;

	for (const auto& z : j["working_zone"])
	{
		wzones.push_back(z);
	}

	v.setWorkZones(std::move(wzones));
	std::vector<std::uint32_t> rzones;

	for (const auto& z : j["restricted_zone"])
	{
		rzones.push_back(z);
	}

	v.setRestrictedZones(std::move(rzones));
	return v;
}

rs::Task taskFromJson(std::uint32_t id, const nlohmann::json& j)
{
	rs::Task t(id);
	t.setExternalId(j["id"]);
	t.setCapacity(j["capacity"].get<float>());
	t.setVolume(j["volume"].get<float>());
	t.setMaxsize(j["maxsize"].get<float>());
	t.setStart(taskPointFromJson(j["start"]));
	t.setEnd(taskPointFromJson(j["end"]));
	t.setVip((bool)j["vip"].get<int>());
	std::vector<std::uint16_t> types;

	for (const auto& e : j["types"])
	{
		types.push_back(e);
	}

	std::sort(types.begin(), types.end());
	t.setTypes(std::move(types));
	return t;
}

JsonParser JsonParser::fromString(const std::string& raw)
{
	return JsonParser(nlohmann::json::parse(raw));
}

JsonParser::JsonParser(const nlohmann::json& root)
{
	std::uint32_t zoneId = 0;

	for (const auto& j : root["zones"])
	{
		m_zones.emplace_back(zoneFromJson(zoneId++, j));
	}

	LOG_TRACE << "number of zones " << m_zones.size();
	std::uint32_t vehicleId = 0;

	for (const auto& j : root["vehicles"])
	{
		m_venchiles.emplace_back(vehicleFromJson(vehicleId++, j));
	}

	LOG_TRACE << "number of vehicles " << m_venchiles.size();
	std::uint32_t taskId = 0;

	for (const auto& j : root["tasks"])
	{
		m_tasks.emplace_back(taskFromJson(taskId++, j));
	}

	LOG_TRACE << "number of tasks " << m_tasks.size();
	const auto& settings = root["settings"];

	if (settings.empty())
	{
		return;
	}

	LOG_TRACE << "settings " << settings;
	auto get_if_found = [&](const std::string & key, auto & val)
	{
		auto it = settings.find(key);

		if (it == settings.end())
		{
			return;
		}

		val = *it;
	};
	get_if_found("mean_track_speed_km_in_hour", m_speedAprox);
	m_speedAprox = (m_speedAprox * 1000.0) / (60.0 * 60.0);
	get_if_found("colony_size_limit", m_abcParams.colonySize);
	get_if_found("trial_count_max_limit", m_abcParams.trialCountMax);
	get_if_found("max_time_in_sec", m_abcParams.maxTimeInSecHardLimit);
	get_if_found("max_inactive_time_in_sec", m_abcParams.maxTimeInSecNoNewBestSolution);
	get_if_found("partial_solution_enabled", m_partialSolutionEnabled);
	get_if_found("log_level", m_logLevel);
	get_if_found("multi_threaded", m_multiThreaded);
	m_accountId = root["account_id"];
}

double JsonParser::getSpeedAprox() const
{
	return m_speedAprox;
}

std::string JsonParser::getLogLevel() const
{
	return m_logLevel;
}

void JsonParser::setTasks(const std::vector<rs::Task>& tasks)
{
	m_tasks = tasks;
}

const ABCParams& JsonParser::getAbcParams() const
{
	return m_abcParams;
}

std::uint32_t JsonParser::getAccountId() const
{
	return m_accountId;
}

bool JsonParser::getPartialSolutionEnabled() const
{
	return m_partialSolutionEnabled;
}

bool JsonParser::getMultiThreaded() const
{
	return m_multiThreaded;
}

const std::vector<rs::Vehicle>& JsonParser::getVenchiles() const
{
	return m_venchiles;
}

const std::vector<rs::Zone>& JsonParser::getZones() const
{
	return m_zones;
}

const std::vector<rs::Task>& JsonParser::getTasks() const
{
	return m_tasks;
}

void JsonParser::produceInputDataCsvToFolder(const std::string& folder) const
{
	using namespace boost::filesystem;

	if (!boost::filesystem::exists(folder))
	{
		auto s = boost::format("folder %s doesn't exists") % folder;
		BOOST_THROW_EXCEPTION(std::runtime_error(s.str()));
	}

	path p(folder);
	std::map<std::uint32_t, std::vector<rs::point2d>> pointsByZones;

	for (const auto& z : m_zones)
	{
		for (const auto& pt : z.getPoints())
		{
			pointsByZones[z.getId()].push_back(pt);
		}
	}

	for (const auto& v : m_venchiles)
	{
		std::ofstream of;
		of.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		auto v_file = (boost::format("venchile_%s.csv") % std::to_string(v.getId())).str();
		of.open((p / path(v_file)).string());
		of << "lat,lon,name" << std::endl;

		for (const auto& zoneId : v.getWorkZonesExternalIds())
		{
			for (const auto& pt : pointsByZones[zoneId])
			{
				of << pt.lat << "," << pt.lon << "," << "wz_" << zoneId << std::endl;
			}
		}
	}

	std::ofstream of;
	of.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	of.open((p / "tasks.csv").string());

	for (const auto& t : m_tasks)
	{
		of << "lat,lon,name" << std::endl;
		of << t.getStart().point.lat << "," << t.getStart().point.lon << ",task_" << t.getExternalId() << "_start" << std::endl;
		of << t.getEnd().point.lat << "," << t.getEnd().point.lon << ",task_" << t.getExternalId() << "_end" << std::endl;
	}

	//std::map<std::uint32_t, workzone> zones;
	//std::vector<venchile> venchiles;
	//std::vector<task> tasks;
}

}
