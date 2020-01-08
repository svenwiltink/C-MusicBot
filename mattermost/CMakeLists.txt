cmake_minimum_required(VERSION 3.13)

file(GLOB Mattermost_SRC
    "*.h"
    "*.c"
)

add_library(mattermost ${Mattermost_SRC})