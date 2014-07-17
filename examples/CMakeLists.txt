set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")

include_directories("${PROJECT_SOURCE_DIR}/include")

include_directories("${PROJECT_SOURCE_DIR}/vendor/asio/asio/include")
add_definitions("-DASIO_STANDALONE")

add_subdirectory(simple-example)
add_subdirectory(http-server-example)
add_subdirectory(router-example)
