cmake_minimum_required(VERSION 3.9.1)
project(SimpleGraphForDisaggregatedMemory)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fopenmp")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/)

add_executable(simple_graph src/main.cc src/graph.cc
    src/sssp.cc src/bfs.cc src/bc.cc src/tc.cc src/pr.cc src/cc.cc src/dfs.cc 
    ext/cpp-arg-parse/src/argparse.cc src/dmalloc.hh)
