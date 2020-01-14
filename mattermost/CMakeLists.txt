cmake_minimum_required(VERSION 3.10)

file(GLOB Mattermost_SRC
    "*.h"
    "*.c"
)

add_library(mattermost ${Mattermost_SRC})

find_library(CURL_LIB curl)
target_link_libraries(mattermost "${CURL_LIB}")

find_library(CJSON_LIB json-c)
target_link_libraries(mattermost "${CJSON_LIB}")

find_library(WEBSOCKETS_LIB websockets)
target_link_libraries(mattermost "${WEBSOCKETS_LIB}")