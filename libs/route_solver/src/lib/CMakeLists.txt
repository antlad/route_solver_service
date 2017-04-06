
set(INCLUDE_FOLDER ../../include/route_solver)
set(INCLUDE_LIST
	${INCLUDE_FOLDER}/solution/bench_loader.hpp
	${INCLUDE_FOLDER}/solution/truck_solution.hpp
	${INCLUDE_FOLDER}/solution/currentroutestate.hpp
	${INCLUDE_FOLDER}/solution/fail_statistics.hpp
	${INCLUDE_FOLDER}/abc/abc.hpp
	${INCLUDE_FOLDER}/abc/abc_common.hpp
	${INCLUDE_FOLDER}/abc/abc_parallel.hpp
	${INCLUDE_FOLDER}/utils/common.hpp
	${INCLUDE_FOLDER}/log.hpp
	${INCLUDE_FOLDER}/structs/data_structs.hpp
	${INCLUDE_FOLDER}/solve/winding_number.hpp
	${INCLUDE_FOLDER}/solve/vehicle_to_task_checker.hpp
	${INCLUDE_FOLDER}/solve/truck_solver.hpp

)

set(SRC_LIST
	utils/common.cpp
	solution/bench_loader.cpp
	solution/truck_solution.cpp
	solution/currentroutestate.cpp
	solution/fail_statistics.cpp
	structs/data_structs.cpp
	solve/winding_number.cpp
	solve/vehicle_to_task_checker.cpp
	solve/truck_solver.cpp
	log.cpp
)

add_library(route_solver ${SRC_LIST} ${INCLUDE_LIST})
if (CMAKE_COMPILER_IS_GNUCXX)
	set_target_properties(route_solver PROPERTIES
		COMPILE_FLAGS "-fPIC"
	)
endif()

link_directories(${Boost_LIBRARY_DIRS})
target_link_libraries(
${PROJECT_NAME}
${Boost_LIBRARIES}
pthread
)
