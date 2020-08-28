cmake_minimum_required(VERSION 3.10)

include(ExternalProject)

ExternalProject_Add(bwa
        GIT_REPOSITORY "https://github.com/bwa-mem2/bwa-mem2.git"
        GIT_TAG v2.0
        SOURCE_DIR "${BWA_ROOT}/bwa"
        BINARY_DIR "${BWA_ROOT}/build"
        INSTALL_DIR "${BWA_ROOT}/install"
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${BWA_ROOT}/install -DWITH_GFLAGS=OFF)