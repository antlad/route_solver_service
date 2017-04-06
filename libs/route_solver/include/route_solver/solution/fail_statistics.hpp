#pragma once

#include <boost/preprocessor.hpp>

#include <vector>
#include <string>

namespace rs {

#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)    \
	case elem : return BOOST_PP_STRINGIZE(elem);

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)                \
	enum name {                                                               \
		BOOST_PP_SEQ_ENUM(enumerators)                                        \
	};                                                                        \
 \
	inline const char* FailEventToString(name v) \
	{                                                                         \
		switch (v)                                                            \
		{                                                                     \
			BOOST_PP_SEQ_FOR_EACH(                                            \
				X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,          \
				name,                                                         \
				enumerators                                                   \
			)                                                                 \
			default: return "[Unknown " BOOST_PP_STRINGIZE(name) "]";         \
		}                                                                     \
	}

DEFINE_ENUM_WITH_STRING_CONVERSIONS(FailEvent,
									(TaskUnLoadBeforeLoad)
									(RouteNodesEmpty)
									(ImpossibleTaskToVenchile)
									(EmptyTasksDuringLoad)
									(MissedTimeFrameDuringLoad)
									(ExceededMaxVolumeDuringLoad)
									(ExceededMaxCapacityDuringLoad)
									(ExceededMaxTasksDuringLoad)
									(EmptyTasksDuringUnload)
									(MissedTimeFrameDuringUnload)
									(ExceededTripDuarationDuringUnload)
									(ExceededTripLengthDuringUnload)
									(CantMoveToEndBeforeStart)
									(MaxElement)
								   )

class FailedStatisticsCollector {
public:
	FailedStatisticsCollector();

	void addEvent(FailEvent e);

	FailedStatisticsCollector& operator += (const FailedStatisticsCollector& other);

	std::string print() const;
private:
	std::vector<std::size_t> m_events;
};

}
