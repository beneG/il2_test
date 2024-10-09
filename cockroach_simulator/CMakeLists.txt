cmake_minimum_required(VERSION 3.17)
project(il2test)

set(CMAKE_CXX_VERSION 17)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator.h 
		${CMAKE_CURRENT_SOURCE_DIR}/Cockroach.h
		${CMAKE_CURRENT_SOURCE_DIR}/Food.h
		${CMAKE_CURRENT_SOURCE_DIR}/Resource.h
		${CMAKE_CURRENT_SOURCE_DIR}/Game.h
		${CMAKE_CURRENT_SOURCE_DIR}/static_global_vars.h
	)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator.cpp) 
	   
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCE}) 



