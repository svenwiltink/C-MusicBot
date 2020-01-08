#ifndef MATTERMOST_H_
#define MATTERMOST_H_ 

struct MatterMostApiOptions {
    char *endpoint;
    char *token;
};

struct MatterMostUser {
    char *id;
    char *username;
    char *nickname;
};


struct MatterMostUser mattermost_get_user_self(struct MatterMostApiOptions options);

#endif