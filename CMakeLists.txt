# Version used - might work with earlier.
cmake_minimum_required(VERSION 3.16)

project(
    Exercises1
)

include_directories(
    .
)

add_compile_options(-g)
add_compile_options(-O0)

add_executable(
    cli 
    cli/main.cpp  
    base/strings.cpp
)
