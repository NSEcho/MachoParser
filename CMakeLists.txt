cmake_minimum_required(VERSION 2.8.12)
project(MachoParser)
 
include_directories(include)

file(GLOB SOURCES "src/*.c")
 
add_executable(MachoParser ${SOURCES})
