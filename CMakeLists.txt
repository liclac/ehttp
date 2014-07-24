cmake_minimum_required(VERSION 2.8)
project(ehttp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
elseif(MSVC)
	# Screw you, I use whatever functions I want :| Security is not done by
	# "deprecating" everything that's even remotely possible to use wrong
	add_definitions(/D_SCL_SECURE_NO_WARNINGS=1)
endif()

add_subdirectory(src)

# Don't include examples, tests and docs if we're a subproject of another
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	add_subdirectory(examples)
	
	enable_testing()
	add_subdirectory(test)

	if(${CMAKE_GENERATOR} MATCHES ".* Makefiles")
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
	endif()

	# Automatically build Doxygen documentation if Doxygen is installed
	find_package(Doxygen)
	if(DOXYGEN_FOUND)
		add_custom_target(docs ALL
			${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile > /dev/null
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			VERBATIM
		)
		
		# Make sure to generate documentation *after* the source targets; this
		# keeps Doxygen from slowing down the build and flipping over invalid code
		add_dependencies(docs ehttp)
	endif(DOXYGEN_FOUND)
endif()
