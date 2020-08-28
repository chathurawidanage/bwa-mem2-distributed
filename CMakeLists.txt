cmake_minimum_required(VERSION 2.8)
project(BWAD)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_REQUIRED_FLAGS "-std=c++17")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# cmake modules directories
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/" ${CMAKE_MODULE_PATH})
list(APPEND CMAKE_MODULE_PATH ${BWA_SOURCE_DIR}/CMake)

include(ConfigureBwa)
include_directories(SYSTEM "${BWA_INCLUDE_DIR}")

# Find MPI
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})

add_executable(bwa-distributed main.cpp)
target_link_libraries(bwa-distributed ${BWA_LIBRARIES} z)