cmake_minimum_required(VERSION 2.8)
project(ehttp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

add_subdirectory(src)
add_subdirectory(examples)

enable_testing()
add_subdirectory(test)

# Add a custom 'make check' target to get output when a test fails
# Taken from http://stackoverflow.com/questions/16158091/
if (CMAKE_CONFIGURATION_TYPES)
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
		--force-new-ctest-process --output-on-failure 
		--build-config "$<CONFIGURATION>")
else()
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
		--force-new-ctest-process --output-on-failure)
endif()
