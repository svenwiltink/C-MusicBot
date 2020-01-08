cmake_minimum_required(VERSION 3.13)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set the project name
project(MusicBot)

# add the executable
add_executable(MusicBot main.c)

find_library(CURL_LIB curl)
target_link_libraries(MusicBot "${CURL_LIB}")