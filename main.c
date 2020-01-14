#include <stdio.h>
#include <stdlib.h>

#include <mattermost.h>

void DummyHandler(struct MatterMostSession *session, struct MatterMostEvent event)
{
  printf("session %s\ndata %s\n", session->name, event.data);
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

  if (mattermost_get_user_self(user, apiOptions)) {
    printf("something went wrong, whoops");
    exit(1);
  }

  printf("ID: %s\n", user->id);
  printf("Username: %s\n", user->username);
  printf("Nickname: %s\n", user->nickname);

  mattermost_free_user(user);

  struct MatterMostSession session = {
      "test1",
      apiOptions,
      NULL,
      NULL,
      DummyHandler};

  mattermost_connect(&session, apiOptions);

  time_t lastTime;
  time_t newTime;
  while (1)
  {
    lws_service(session.lws_context, 250);

    time(&newTime);
    if (newTime > lastTime)
    {
      lastTime = newTime;
      lws_callback_on_writable(session.lws_websocket);
    }
  }

  mattermost_session_free(&session);
}