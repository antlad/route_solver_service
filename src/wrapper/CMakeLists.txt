project(wrapper)

set(SRC_LIST
	interface.cpp
	../../include/wrapper/interface.hpp
	)

link_directories(${Boost_LIBRARY_DIRS})
add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
target_link_libraries(
	${PROJECT_NAME}
	create_route
	${Boost_LIBRARIES}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	install(TARGETS ${PROJECT_NAME}
	DESTINATION /usr/local/lib
	)
endif()
