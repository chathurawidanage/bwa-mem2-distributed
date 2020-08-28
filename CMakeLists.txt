cmake_minimum_required(VERSION 3.16)
project(bwa-distributed)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

# Find MPI
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_CXX_INCLUDE_PATH})

add_executable(bwa-distributed main.cpp)