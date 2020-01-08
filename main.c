#include <stdio.h>
#include <stdlib.h>

#include <mattermost.h>

int main() {

  char *token = getenv("MusicBot_Mattermost_Token");
  if (token == NULL) {
    printf("MusicBot_Mattermost_Token env variable not set");
    return 1;
  }

  char *endpoint = getenv("MusicBot_Mattermost_Endpoint");
  if (endpoint == NULL) {
    printf("MusicBot_Mattermost_Endpoint env variable not set");
    return 1;
  }

  printf("Found mattermost token %s", token);

  struct MatterMostApiOptions apiOptions = {
    endpoint,
    token
  };

  mattermost_get_user_self(apiOptions);
}