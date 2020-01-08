cmake_minimum_required(VERSION 3.13)

file(GLOB Mattermost_SRC
    "*.h"
    "*.c"
)

find_library(CURL_LIB curl)
target_link_libraries(MusicBot "${CURL_LIB}")

add_library(mattermost ${Mattermost_SRC})