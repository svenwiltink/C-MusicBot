#ifndef MATTERMOST_H_
#define MATTERMOST_H_ 

typedef struct ApiOptions {
    char *endpoint;
    char *token;
} MatterMostApiOptions;

MatterMostApiOptions mattermost_get_api_options();

#endif