cmake_minimum_required(VERSION 2.8)
project(bwa-distributed)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_REQUIRED_FLAGS "-std=c++17")

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# Find MPI
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})

add_executable(bwa-distributed main.cpp)