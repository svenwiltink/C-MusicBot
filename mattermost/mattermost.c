#include "mattermost.h"

MatterMostApiOptions mattermost_get_api_options() {
    callCurl();
    MatterMostApiOptions options = {"lekkere", "banaan"};
    return options;
}