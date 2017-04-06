
cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
	message("Using ccahe!")
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

option(ENABLE_UNIT_TESTS OFF)
option(ENABLE_BACKEND_BUILD OFF)
option(BUILD_ENABLE_CPACK OFF)

add_definitions(-DENABLE_LOG)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
add_definitions(-DBOOST_LOG_DYN_LINK=1)
set(CMAKE_CXX_STANDARD 14)
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)

find_package(Boost REQUIRED COMPONENTS filesystem program_options system log program_options thread log_setup date_time thread)
find_package(LibOSRM REQUIRED)
find_package(Eigen3 REQUIRED)
include_directories(
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/libs/json
	${CMAKE_CURRENT_LIST_DIR}/libs/route_solver/include
	${LibOSRM_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
	${EIGEN3_INCLUDE_DIR}
)

add_subdirectory(src/console)
add_subdirectory(src/lib)
add_subdirectory(libs/route_solver)
add_subdirectory(src/wrapper)
if(ENABLE_UNIT_TESTS)
	add_subdirectory(tests)
endif()


if(ENABLE_BACKEND_BUILD)
	find_package(Go 1.5 REQUIRED)
	set (CGO_FLAGS "CGO_LDFLAGS=-L${CMAKE_CURRENT_BINARY_DIR}/src/wrapper")
	set (GOPATH "GOPATH=${CMAKE_SOURCE_DIR}/golang")
	set (SETUP_COMMAND export ${GOPATH}&& export ${CGO_FLAGS})
	add_custom_command(
			OUTPUT create_route_backend
			COMMAND ${SETUP_COMMAND} && go get .
			COMMAND ${SETUP_COMMAND} && go build -o "${CMAKE_BINARY_DIR}/golang/create_route_backend"
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/golang/src/backend
			DEPENDS create_route wrapper)

	add_custom_target(create_route_backend_run ALL DEPENDS create_route_backend create_route wrapper)

	if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(PROGRAMS "${CMAKE_BINARY_DIR}/golang/create_route_backend" DESTINATION /usr/local/bin)
	endif()
endif()

install(
	PROGRAMS
		/usr/local/bin/osrm-contract
		/usr/local/bin/osrm-datastore
		/usr/local/bin/osrm-extract
		/usr/local/bin/osrm-routed
	DESTINATION
		/usr/local/bin)

if(BUILD_ENABLE_CPACK AND (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
	set (VERSION "0.1.3")
	set(CPACK_PACKAGE_VERSION ${VERSION})
	set(CPACK_DEBIAN_PACKAGE_VERSION ${VERSION})
	set(CPACK_PACKAGE_NAME "create_route")
	set(CPACK_GENERATOR "DEB")
	set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
	set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Vladislav Troinich")

	set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

	set(CPACK_PACKAGE_DESCRIPTION "create_route short description")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "create_route long description")
	set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
	include(CPack)
endif()


