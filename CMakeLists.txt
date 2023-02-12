# Version used - might work with earlier.
cmake_minimum_required(VERSION 3.16)

project(
    string-class-example
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

include_directories(
    .
)

add_compile_options(-g)
add_compile_options(-Og)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wparentheses)
add_compile_options(-Werror)
# add_compile_options(-pendantic)

add_executable(
    cli 
    cli/main.cpp  
    base/strings.cpp
    base/threads.cpp
)

add_executable(
    b0
    b1/populate_optional.cpp
)
# c++ -std=c++17 -O3 populate_optional.cpp -o populate_optional -Wall -Wextra -O3 -fno-tree-vectorize
target_compile_options(
    b0 PRIVATE 
    -O3 -fno-tree-vectorize
)

add_executable(
    b1
    b1/main.cpp
    base/strings.cpp
)
target_compile_options(
    b1 PRIVATE 
    -O3
)
