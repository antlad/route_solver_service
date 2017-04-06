#include "route_solver/log.hpp"
#include "route_solver/structs/data_structs.hpp"
#include "route_solver/solve/winding_number.hpp"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>

namespace rs {

using boost::lexical_cast;
using boost::format;

bool isPointInZone(const point2d& pt, const std::vector<point2d>& workzone)
{
	return wn_PnPoly(pt, workzone) != 0;
}

namespace {

bool isPointInCirle(const point2d& pt, const point2d& c, double r)
{
	double x1 = pt.x();
	double y1 = pt.y();
	double x2 = c.x();
	double y2 = c.y();
	double len = (double) sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	return len < r;
}

}

bool Zone::isTaskInZone(const Task& task) const
{
	bool first_point = false;
	bool second_point = false;

	if (m_zoneType == Zone::Circle)
	{
		first_point = isPointInCirle(task.getStart().point, *m_circleCenter, m_circleRadius);
		second_point = isPointInCirle(task.getEnd().point, *m_circleCenter, m_circleRadius);
	}
	else if (m_zoneType == Zone::Polygon)
	{
		first_point = isPointInZone(task.getStart().point, m_points);
		second_point = isPointInZone(task.getEnd().point, m_points);
	}
	else
	{
		BOOST_THROW_EXCEPTION(std::runtime_error("Unknow zone type"));
	}

	return first_point && second_point;
}

std::uint32_t Zone::getExternalId() const
{
	return m_externalId;
}

void Zone::setExternalId(const std::uint32_t& externalId)
{
	m_externalId = externalId;
}

void Zone::setPolygonPoints(std::vector<point2d>&& points)
{
	m_points = std::move(points);
	m_zoneType = Zone::Polygon;
}

void Zone::setCircle(double circleRadius, const point2d& circleCenter)
{
	m_circleCenter = circleCenter;
	m_circleRadius = circleRadius;
	m_zoneType = Zone::Circle;
}

Zone::Zone(std::uint32_t id)
	: m_id(id)
	, m_externalId(0)
	, m_circleRadius(0)
	, m_zoneType(ZoneType::Circle)
{
}

std::uint32_t Zone::getId() const
{
	return m_id;
}

Zone::ZoneType Zone::getZoneType() const
{
	return m_zoneType;
}

const std::vector<point2d>& Zone::getPoints() const
{
	return m_points;
}

Vehicle::Vehicle(std::uint32_t id)
	: m_id(id)
	, m_externalId(0)
	, m_tripMaxTasks(std::numeric_limits<std::uint32_t>::max())
	, m_tripDuration(std::numeric_limits<double>::max())
	, m_tripLength(std::numeric_limits<double>::max())
	, m_capacity(std::numeric_limits<float>::max())
	, m_length(std::numeric_limits<float>::max())
	, m_height(std::numeric_limits<float>::max())
	, m_width(std::numeric_limits<float>::max())
	, m_costOfUsing(1.0)
	, m_costPerKM(false)
{}


std::uint32_t Vehicle::getId() const
{
	return m_id;
}

float Vehicle::getCapacity() const
{
	return m_capacity;
}

float Vehicle::getLength() const
{
	return m_length;
}

float Vehicle::getHeight() const
{
	return m_height;
}

float Vehicle::getWidth() const
{
	return m_width;
}

float Vehicle::getMaxVolume() const
{
	return m_width * m_height * m_length;
}

float Vehicle::getTripDuration() const
{
	return m_tripDuration;
}

float Vehicle::getTripLength() const
{
	return m_tripLength;
}


const std::vector<uint32_t>& Vehicle::getWorkZonesExternalIds() const
{
	return m_workZones;
}

float Vehicle::getCostOfUsing() const
{
	return m_costOfUsing;
}

std::uint16_t Vehicle::getTripMaxTasks() const
{
	return m_tripMaxTasks;
}

const std::vector<std::uint32_t>& Vehicle::getRestrictedZonesExternalIds() const
{
	return m_restrictedZones;
}

Task::Task(uint32_t id)
	: m_id(id)
	, m_externalId(0)
	, m_vip(false)
	, m_capacity(0)
	, m_volume(0)
	, m_maxsize(0)
{
}

std::uint32_t Task::getId() const
{
	return m_id;
}

float Task::getCapacity() const
{
	return m_capacity;
}

float Task::getVolume() const
{
	return m_volume;
}

float Task::getMaxSize() const
{
	return m_maxsize;
}

bool Task::getVip() const
{
	return m_vip;
}

const std::vector<uint16_t>& Task::getTypes() const
{
	return m_types;
}

const TaskPoint& Task::getStart() const
{
	return m_start;
}

const TaskPoint& Task::getEnd() const
{
	return m_end;
}

std::uint32_t Task::getExternalId() const
{
	return m_externalId;
}

void Task::setExternalId(const std::uint32_t& externalId)
{
	m_externalId = externalId;
}

void Task::setVip(bool vip)
{
	m_vip = vip;
}

void Task::setCapacity(float capacity)
{
	m_capacity = capacity;
}

void Task::setVolume(float volume)
{
	m_volume = volume;
}

void Task::setMaxsize(float maxsize)
{
	m_maxsize = maxsize;
}

void Task::setStart(TaskPoint&& start)
{
	m_start = std::move(start);
}

void Task::setEnd(TaskPoint&& end)
{
	m_end = std::move(end);
}

void Task::setTypes(std::vector<std::uint16_t>&& types)
{
	m_types = std::move(types);
}

//TODO: chcek this https://www.codeproject.com/questions/626899/converting-latitude-and-longitude-to-an-x-y-coordi

/*
R = 6371;
from https://www.codeproject.com/questions/626899/converting-latitude-and-longitude-to-an-x-y-coordi
x = R * cos(lat) * cos(lon)
y = R * cos(lat) * sin(lon)
z = R *sin(lat)
*/

point2d::point2d(double lat, double lon)
	: lat(lat), lon(lon)
{
}

double point2d::x() const
{
	return lon;
}

double point2d::y() const
{
	return lat;
}


bool rs::Vehicle::canTakeTask(const rs::Task& task) const
{
	for (const auto& t : task.getTypes())
	{
		if (!std::binary_search(m_types.begin(), m_types.end(), t))
		{
			return false;
		}
	}

	if (m_capacity < task.getCapacity())
	{
		return false;
	}

	if (m_width * m_length * m_height < task.getVolume())
	{
		return false;
	}

	if (m_width < task.getMaxSize() && m_height < task.getMaxSize())
	{
		return false;
	}

	return true;
}

const std::vector<uint16_t>& Vehicle::getTypes() const
{
	return m_types;
}

std::uint32_t Vehicle::getExternalId() const
{
	return m_externalId;
}

bool Vehicle::costPerKM() const
{
	return m_costPerKM;
}

void Vehicle::setExternalId(uint32_t externalId)
{
	m_externalId = externalId;
}

void Vehicle::setId(uint32_t id)
{
	m_id = id;
}

void Vehicle::setCapacity(float capacity)
{
	m_capacity = capacity;
}

void Vehicle::setLength(float length)
{
	m_length = length;
}

void Vehicle::setHeight(float height)
{
	m_height = height;
}

void Vehicle::setWidth(float width)
{
	m_width = width;
}

void Vehicle::setTripDuration(double tripDuration)
{
	m_tripDuration = tripDuration;
}

void Vehicle::setTripLength(double tripLength)
{
	m_tripLength = tripLength;
}

void Vehicle::setTypes(const std::vector<uint16_t>& types)
{
	m_types = types;
}

std::vector<std::uint32_t> Vehicle::getWorkZones() const
{
	return m_workZones;
}

void Vehicle::setWorkZones(std::vector<std::uint32_t>&& workZones)
{
	m_workZones = std::move(workZones);
}

std::vector<std::uint32_t> Vehicle::getRestrictedZones() const
{
	return m_restrictedZones;
}

void Vehicle::setRestrictedZones(std::vector<std::uint32_t>&& restrictedZones)
{
	m_restrictedZones = std::move(restrictedZones);
}

void Vehicle::setCostOfUsing(float costOfUsing)
{
	m_costOfUsing = costOfUsing;
}

bool Vehicle::getCostPerKM() const
{
	return m_costPerKM;
}

void Vehicle::setCostPerKM(bool costPerKM)
{
	m_costPerKM = costPerKM;
}

void Vehicle::setTripMaxTasks(uint32_t tripMaxTasks)
{
	m_tripMaxTasks = tripMaxTasks;
}

TaskPoint::TaskPoint(point2d&& point, std::size_t ts, std::size_t te, std::size_t dur)
	: point(std::move(point))
	, ts(ts)
	, te(te)
	, dur(dur)
{
}

}
