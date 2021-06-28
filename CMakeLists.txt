cmake_minimum_required(VERSION 3.3)
project(datachannelTest LANGUAGES CXX)

add_executable(datachannelTest main.cpp simpleDatachannel.h simpleDatachannel.cpp)

set_target_properties(datachannelTest PROPERTIES CXX_STANDARD 17)

target_include_directories(
		datachannelTest
		PUBLIC
		${PROJECT_SOURCE_DIR}/include/rtc
		${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
		datachannelTest ${PROJECT_SOURCE_DIR}/datachannel.lib
)
