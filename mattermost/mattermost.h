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

int mattermost_get_user_self(struct MatterMostUser *user, struct MatterMostApiOptions options);

void mattermost_free_user(struct MatterMostUser *user);

void mattermost_connect(struct MatterMostApiOptions options);

#endif