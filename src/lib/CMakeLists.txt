project(create_route)
set(CMAKE_CXX_FLAGS ${LibOSRM_CXXFLAGS})

# TODO: remove this agly
set(INCLUDE_LIST
	../../include/parser/json_parser.hpp
	../../include/solve/solver.hpp
	)

set(SRC_LIST
	parser/json_parser.cpp
	solve/solver.cpp
	)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST} ${INCLUDE_LIST})
link_directories(${LibOSRM_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS})
target_link_libraries(
	${PROJECT_NAME}
	${LibOSRM_LIBRARIES}
	${LibOSRM_DEPENDENT_LIBRARIES}
	${Boost_LIBRARIES}
	route_solver
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(TARGETS ${PROJECT_NAME}
	DESTINATION /usr/local/lib
	)
endif()
