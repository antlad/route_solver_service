//
// Created by vlad on 14.11.16.
//

#pragma once

#include <boost/optional.hpp>

#include <vector>
#include <memory>

namespace rs {

struct point2d
{
	point2d() = default;

	point2d(double lat,
			double lon);

	double x() const;

	double y() const;

	double lat;
	double lon;
};

struct TaskPoint
{
	TaskPoint() = default;
	TaskPoint(point2d&& point,
			  std::size_t ts,
			  std::size_t te,
			  std::size_t dur);

	point2d point;
	std::size_t ts = 0;
	std::size_t te = 0;
	std::size_t dur = 0;
};

class Task {
public:
	Task(std::uint32_t id);

	std::uint32_t getId() const;

	float getCapacity() const;
	void setCapacity(float capacity);

	float getVolume() const;
	void setVolume(float volume);

	float getMaxSize() const;
	void setMaxsize(float maxsize);

	bool getVip() const;
	void setVip(bool vip);

	const std::vector<std::uint16_t>& getTypes() const;
	void setTypes(std::vector<std::uint16_t>&& types);

	const TaskPoint& getStart() const;
	void setStart(TaskPoint&& start);

	const TaskPoint& getEnd() const;
	void setEnd(TaskPoint&& end);

	std::uint32_t getExternalId() const;
	void setExternalId(const std::uint32_t& externalId);

private:
	std::uint32_t m_id;
	std::uint32_t m_externalId;
	bool m_vip;
	float m_capacity;
	float m_volume;
	float m_maxsize;
	TaskPoint m_start;
	TaskPoint m_end;
	std::vector<std::uint16_t> m_types;
};

bool isPointInZone(const point2d& pt, const std::vector<point2d>& workzone);

class Zone {
public:
	enum ZoneType
	{
		Circle,
		Polygon
	};

	Zone(std::uint32_t id);

	std::uint32_t getId() const;

	ZoneType getZoneType() const;

	bool isTaskInZone(const Task& task) const;

	const std::vector<point2d>& getPoints() const;

	std::uint32_t getExternalId() const;

	void setExternalId(const std::uint32_t& externalId);

	void setPolygonPoints(std::vector<point2d>&& points);

	void setCircle(double circleRadius, const point2d& circleCenter);

private:
	std::uint32_t m_id;
	std::uint32_t m_externalId;
	double m_circleRadius;
	ZoneType m_zoneType;
	boost::optional<point2d> m_circleCenter;
	std::vector <point2d> m_points;
};

class Vehicle {
public:
	Vehicle(std::uint32_t id);

	std::uint32_t getExternalId() const;
	void setExternalId(std::uint32_t externalId);

	std::uint32_t getId() const;
	void setId(std::uint32_t id);

	float getCapacity() const;
	void setCapacity(float capacity);

	float getLength() const;
	void setLength(float length);

	float getHeight() const;
	void setHeight(float height);

	float getWidth() const;
	void setWidth(float width);

	float getTripDuration() const;
	void setTripDuration(double tripDuration);

	float getTripLength() const;
	void setTripLength(double tripLength);


	std::vector<std::uint32_t> getWorkZones() const;
	void setWorkZones(std::vector<std::uint32_t>&& workZones);

	std::vector<std::uint32_t> getRestrictedZones() const;
	void setRestrictedZones(std::vector<std::uint32_t>&& restrictedZones);

	float getCostOfUsing() const;
	void setCostOfUsing(float costOfUsing);

	bool getCostPerKM() const;
	void setCostPerKM(bool costPerKM);

	std::uint16_t getTripMaxTasks() const;
	void setTripMaxTasks(std::uint32_t tripMaxTasks);

	const std::vector<std::uint32_t>& getWorkZonesExternalIds() const;
	const std::vector<std::uint32_t>& getRestrictedZonesExternalIds() const;

	const std::vector<uint16_t>& getTypes() const;
	void setTypes(const std::vector<uint16_t>& types);

	bool costPerKM() const;
	bool canTakeTask(const Task& task) const;
	float getMaxVolume() const;

private:
	std::uint32_t m_id;
	std::uint32_t m_externalId;
	std::uint32_t m_tripMaxTasks;
	double m_tripDuration;
	double m_tripLength;
	float m_capacity;
	float m_length;
	float m_height;
	float m_width;
	float m_costOfUsing;
	bool m_costPerKM;
	std::vector<uint16_t> m_types;
	std::vector<std::uint32_t> m_workZones;
	std::vector<std::uint32_t> m_restrictedZones;
};


}
