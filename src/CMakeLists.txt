set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

#
# --> Find Libraries
#
include_directories("${PROJECT_SOURCE_DIR}/vendor/http-parser")
set(http_parser_SOURCES "${PROJECT_SOURCE_DIR}/vendor/http-parser/http_parser.c")

include_directories("${PROJECT_SOURCE_DIR}/vendor/asio/asio/include")
add_definitions("-DASIO_STANDALONE")

#
# --> Build Library
#
include_directories("${PROJECT_SOURCE_DIR}/include")
file(GLOB_RECURSE ehttp_HEADERS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE ehttp_SOURCES "*.cpp")
add_library(ehttp ${ehttp_SOURCES} ${ehttp_HEADERS} ${http_parser_SOURCES})
