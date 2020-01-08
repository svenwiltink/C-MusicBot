#include <stdio.h>
#include <mattermost.h>

int main() {
  MatterMostApiOptions options = mattermost_get_api_options();
  printf("%s", options.token);
  printf("%s", options.endpoint);
}