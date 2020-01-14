#ifndef MATTERMOST_H_
#define MATTERMOST_H_

struct MatterMostApiOptions
{
    char *endpoint;
    char *token;
};

struct MatterMostUser
{
    char *id;
    char *username;
    char *nickname;
};

struct MatterMostSession;

struct MatterMostEvent
{
    char *data;
};

typedef void (*MatterMostHandleEvent)(struct MatterMostSession *session, struct MatterMostEvent event);

typedef struct MatterMostSession
{
    char name[20];
    MatterMostHandleEvent eventhandler;
} MatterMostSession;

int mattermost_get_user_self(struct MatterMostUser *user, struct MatterMostApiOptions options);

void mattermost_free_user(struct MatterMostUser *user);

void mattermost_connect(struct MatterMostApiOptions options);

#endif