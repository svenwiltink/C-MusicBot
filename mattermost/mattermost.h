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

typedef struct MatterMostSession
{
    struct MatterMostApiOptions apiOptions;
    MatterMostHandleEvent eventhandler;
    enum MatterMostSessionStates state;
    struct lws_context *lws_context;
    struct lws *lws_websocket;
    time_t lastPing;
} MatterMostSession;

int mattermost_get_user_self(struct MatterMostUser *user, struct MatterMostApiOptions options);

void mattermost_free_user(struct MatterMostUser *user);

void mattermost_init(struct MatterMostSession *session, struct MatterMostApiOptions apiOptions);

void mattermost_set_eventhandler(struct MatterMostSession *session, MatterMostHandleEvent eventHandler);

void mattermost_connect(struct MatterMostSession *session, struct MatterMostApiOptions options);

void mattermost_service(struct MatterMostSession *session);

void mattermost_session_free(struct MatterMostSession *session);

#endif