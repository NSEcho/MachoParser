cmake_minimum_required(VERSION 1.0.1)
project(parser)
 
include_directories(include)

file(GLOB SOURCES "src/*.c")
 
add_executable(parser ${SOURCES})
