# Version used - likely works with earlier.
cmake_minimum_required(VERSION 3.16)

project(
    string-class-example
)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

file(MAKE_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_custom_target(
    VERSION_BUMP ALL 
    COMMAND /usr/bin/bash ${PROJECT_SOURCE_DIR}/mkversion.sh ${PROJECT_SOURCE_DIR}/version.h
)

include_directories(
    .
)

# add_compile_options(-g)
# add_compile_options(-Og)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wparentheses)
add_compile_options(-Werror)
# add_compile_options(-Wpedantic)

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
add_executable(
    b1
    b1/main.cpp
    base/strings.cpp
)
add_executable(
    b2
    b2/main.cpp
    base/strings.cpp
    samples/samples.cpp
)

target_compile_options(cli PRIVATE -g -Og)
target_compile_options(b0 PRIVATE -O3)
target_compile_options(b1 PRIVATE -O3)
# target_compile_options(b2 PRIVATE -g -Og)
target_compile_options(b2 PRIVATE -O3)
