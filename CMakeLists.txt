cmake_minimum_required(VERSION 3.17)
project(il2test)

set(CMAKE_CXX_VERSION 17)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator/cockroach_simulator.h 
		${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator/Cockroach.h
		${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator/Food.h
		${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator/Resource.h
		${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator/Game.h
		${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator/static_global_vars.h
	)

${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_PROJECT_NAME}.sln

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cockroach_simulator/cockroach_simulator.cpp) 
	   
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCE}) 



