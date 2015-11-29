cmake_minimum_required(VERSION 3.3)
project(terminition)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG  "-Wall -fno-rtti -O0 -g")

set(SOURCE_FILES main.cpp searchTree/Node.cpp searchTree/Common.h loopAnalysis/termLoopPass.cpp searchTree/Common.cpp)
add_executable(terminition ${SOURCE_FILES})

include_directories(${LLVM_INCLUDE_DIRS} .)
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs
        support core irreader executionengine interpreter
        mc mcjit bitwriter x86codegen target)
target_link_libraries(terminition ${llvm_libs})

message(STATUS "XXF ${llvm_libs}")