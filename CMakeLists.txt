cmake_minimum_required(VERSION 3.10)
project(F1PowerUnit)

include_directories(include)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC_CPP CONFIGURE_DEPENDS "src/*.cpp")

add_executable(f1-pu ${SRC_CPP})

target_include_directories(f1-pu PRIVATE include)
