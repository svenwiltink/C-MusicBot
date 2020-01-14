cmake_minimum_required(VERSION 3.10)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# set the project name
project(MusicBot)

file(GLOB MusicBot_SRC
    "*.h"
    "*.c"
)

# add the executable
add_executable(MusicBot ${MusicBot_SRC})

include_directories(mattermost)
add_subdirectory(mattermost)
target_link_libraries(MusicBot mattermost)