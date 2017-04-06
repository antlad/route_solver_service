project(route_solver)
cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
	message("Using ccache!")
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

option(ENABLE_UNIT_TESTS OFF)


add_definitions(-DBOOST_LOG_DYN_LINK=1)
set(CMAKE_CXX_STANDARD 14)

find_package(Boost REQUIRED COMPONENTS filesystem program_options system log program_options thread log_setup date_time)

find_package(Eigen3 REQUIRED)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${Boost_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
)


add_subdirectory(src/console)
add_subdirectory(src/lib)

if(ENABLE_UNIT_TESTS)
    add_subdirectory(unit_tests)
endif()


