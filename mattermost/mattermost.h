#ifndef MATTERMOST_H_
#define MATTERMOST_H_

#include <libwebsockets.h>

struct MatterMostSession;

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

enum MatterMostEventType
{
    MATTERMOST_EVENT_TYPE_POSTED
};

enum MatterMostChannelType
{
    MATTERMOST_CHANNEL_TYPE_DIRECT,
    MATTERMOST_CHANNEL_TYPE_CHANNEL
};

struct MatterMostEvent
{
    enum MatterMostEventType type;
    void *data;
};

struct MatterMostChannel
{
    char *displayname;
    char *name;
    enum MatterMostChannelType type;
};

struct MatterMostEventPosted
{
    struct MatterMostUser user;
    struct MatterMostChannel channel;
};

typedef void (*MatterMostHandleEvent)(struct MatterMostSession *session, enum MatterMostEventType eventType, void *event);

int mattermost_get_user_self(struct MatterMostUser *user, struct MatterMostApiOptions options);

void mattermost_free_user(struct MatterMostUser *user);

struct MatterMostSession *mattermost_init(struct MatterMostApiOptions apiOptions);

int mattermost_get_state(struct MatterMostSession *session);

void mattermost_set_eventhandler(struct MatterMostSession *session, MatterMostHandleEvent eventHandler);

void mattermost_connect(struct MatterMostSession *session, struct MatterMostApiOptions options);

void mattermost_service(struct MatterMostSession *session);

void mattermost_session_free(struct MatterMostSession *session);

#endif