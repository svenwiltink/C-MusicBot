#ifndef MATTERMOST_H_
#define MATTERMOST_H_

#include <libwebsockets.h>

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

enum MatterMostSessionStates
{
    MATTERMOST_SESSION_CONNECTING,
    MATTERMOST_SESSION_AUTHENTICATING,
    MATTERMOST_SESSION_AUTHENTICATING_WAITING,
    MATTERMOST_SESSION_AUTHENTICATION_FAILED,
    MATTERMOST_SESSION_CONNECTED,
    MATTERMOST_SESSION_DISCONNECTED
};

struct MatterMostSession;

struct MatterMostEvent
{
    char *data;
};

typedef void (*MatterMostHandleEvent)(struct MatterMostSession *session, struct MatterMostEvent event);

int mattermost_get_user_self(struct MatterMostUser *user, struct MatterMostApiOptions options);

void mattermost_free_user(struct MatterMostUser *user);

struct MatterMostSession *mattermost_init(struct MatterMostApiOptions apiOptions);

int mattermost_get_state(struct MatterMostSession *session);

void mattermost_set_eventhandler(struct MatterMostSession *session, MatterMostHandleEvent eventHandler);

void mattermost_connect(struct MatterMostSession *session, struct MatterMostApiOptions options);

void mattermost_service(struct MatterMostSession *session);

void mattermost_session_free(struct MatterMostSession *session);

#endif