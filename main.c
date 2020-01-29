#include <stdio.h>
#include <stdlib.h>

#include <mattermost.h>

void DummyHandler(struct MatterMostSession *session, enum MatterMostEventType eventType, void *event)
{
  switch (eventType)
  {
  case MATTERMOST_EVENT_TYPE_POSTED:
  {
    struct MatterMostEventPosted *post = (struct MatterMostEventPosted *)event;
    printf("got a message in %s\n", post->channel.displayname);
  }
  default:
    return;
  }
}

int main()
{

  char *token = getenv("MusicBot_Mattermost_Token");
  if (token == NULL)
  {
    printf("MusicBot_Mattermost_Token env variable not set");
    return 1;
  }

  char *endpoint = getenv("MusicBot_Mattermost_Endpoint");
  if (endpoint == NULL)
  {
    printf("MusicBot_Mattermost_Endpoint env variable not set");
    return 1;
  }

  printf("Found mattermost token %s", token);

  struct MatterMostApiOptions apiOptions = {
      endpoint,
      token};

  struct MatterMostUser *user = malloc(sizeof(struct MatterMostUser));

  if (mattermost_get_user_self(user, apiOptions))
  {
    printf("something went wrong, whoops");
    exit(1);
  }

  printf("ID: %s\n", user->id);
  printf("Username: %s\n", user->username);
  printf("Nickname: %s\n", user->nickname);

  mattermost_free_user(user);

  struct MatterMostSession *session;

  session = mattermost_init(apiOptions);
  mattermost_set_eventhandler(session, DummyHandler);
  mattermost_connect(session, apiOptions);

  int state;
  while (1)
  {
    state = mattermost_get_state(session);
    if (state == MATTERMOST_SESSION_DISCONNECTED || state == MATTERMOST_SESSION_AUTHENTICATION_FAILED)
    {
      printf("stoppin main loop, reached status %d\n", state);
      break;
    }

    mattermost_service(session);
  }

  mattermost_session_free(session);
}