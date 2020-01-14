#include <stdio.h>
#include <stdlib.h>

#include <mattermost.h>

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

  mattermost_get_user_self(user, apiOptions);

  printf("ID: %s\n", user->id);
  printf("Username: %s\n", user->username);
  printf("Nickname: %s\n", user->nickname);

  mattermost_free_user(user);

  mattermost_connect(apiOptions);
}