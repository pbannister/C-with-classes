# Version used - might work with earlier.
cmake_minimum_required(VERSION 3.16)

project(
    string-class-example
)

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
add_compile_options(-Wparentheses )
add_compile_options(-Werror)
# add_compile_options(-pendantic)

add_executable(
    cli 
    cli/main.cpp  
    base/strings.cpp
    base/threads.cpp
)
