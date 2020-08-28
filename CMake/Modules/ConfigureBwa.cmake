# build bwa from github tag
set(BWA_ROOT ${CMAKE_BINARY_DIR}/bwa)
set(BWA_INSTALL ${CMAKE_BINARY_DIR}/bwa/install)

#if (UNIX)
#    set(BWA_EXTRA_COMPILER_FLAGS "-fPIC")
#endif()

set(BWA_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${BWA_EXTRA_COMPILER_FLAGS})
set(BWA_C_FLAGS ${CMAKE_C_FLAGS} ${BWA_EXTRA_COMPILER_FLAGS})

SET(WITH_GFLAGS OFF)

configure_file("${CMAKE_SOURCE_DIR}/CMake/Templates/Bwa.CMakeLists.txt.cmake"
        "${BWA_ROOT}/CMakeLists.txt")

execute_process(
        COMMAND "git submodule init && git submodule update"
        RESULT_VARIABLE BWA_BUILD
        WORKING_DIRECTORY ${BWA_ROOT}/bwa)

execute_process(
        COMMAND "make"
        RESULT_VARIABLE BWA_BUILD
        WORKING_DIRECTORY ${BWA_ROOT}/bwa)

file(MAKE_DIRECTORY "${BWA_ROOT}/include")

file(GLOB PUBLIC_HEADERS
        "${BWA_ROOT}/bwa/src/*.h"
        )
file(COPY ${PUBLIC_HEADERS} DESTINATION ${BWA_ROOT}/include)

set(BWA_LIBRARIES "${BWA_ROOT}/bwa/libbwa.a" "${BWA_ROOT}/bwa/ext/safestringlib/libsafestring.a")
set(BWA_INCLUDE_DIR "${BWA_ROOT}/include" "${BWA_ROOT}/bwa/ext/safestringlib/include")

set(BWA_FOUND TRUE)
#
#file(GLOB all_bwa_libs
#        "${BWA_ROOT}/bwa/src/*.o"
#        "${BWA_ROOT}/bwa/ext/safestringlib/obj/*.o"
#        )
#
#set(BWA_LIBRARIES "${all_bwa_libs}")
