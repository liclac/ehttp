set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -fPIC")



#
# --> Find Libraries
include_directories("${PROJECT_SOURCE_DIR}/vendor/http-parser")
set(http_parser_SOURCES "${PROJECT_SOURCE_DIR}/vendor/http-parser/http_parser.c")

include_directories("${PROJECT_SOURCE_DIR}/vendor/asio/asio/include")
add_definitions("-DASIO_STANDALONE")



#
# --> Build Library
#
file(GLOB_RECURSE ehttp_SOURCES *.cpp *.h)
add_library(ehttp ${ehttp_SOURCES} ${http_parser_SOURCES})
