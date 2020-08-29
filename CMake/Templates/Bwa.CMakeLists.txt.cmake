cmake_minimum_required(VERSION 2.8)

include(ExternalProject)
project(BWA)
ExternalProject_Add(bwa
        GIT_REPOSITORY "https://github.com/bwa-mem2/bwa-mem2.git"
        GIT_TAG v2.0
        SOURCE_DIR "${BWA_ROOT}/bwa"
        BINARY_DIR "${BWA_ROOT}/build"
        INSTALL_DIR "${BWA_ROOT}/install"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND ""
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${BWA_ROOT}/install -DWITH_GFLAGS=OFF)