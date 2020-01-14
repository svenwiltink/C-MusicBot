#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "mattermost.h"

struct MemoryStruct
{
    char *memory;
    size_t size;
};

int curlInitialized = 0;

void initCurl()
{
    if (!curlInitialized)
    {
        printf("initializing curl");
        curl_global_init(CURL_GLOBAL_ALL);
        curlInitialized = 1;
    }
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL)
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int mattermost_get_user_self(struct MatterMostUser *user, struct MatterMostApiOptions options)
{
    int status = 0;
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;

    chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
    chunk.size = 0;           /* no data at this point */

    initCurl();

    /* init the curl session */
    curl_handle = curl_easy_init();

    size_t needed = snprintf(NULL, 0, "Authorization: Bearer %s", options.token) + 1;
    char *tokenHeader = malloc(needed);
    sprintf(tokenHeader, "Authorization: Bearer %s", options.token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, tokenHeader);

    char *endpoint = malloc(sizeof(options.endpoint) + sizeof("/api/v4/users/me"));
    strcpy(endpoint, options.endpoint);
    strcat(endpoint, "/api/v4/users/me");

    printf("header: %s\n", tokenHeader);
    printf("ENDPOINT: %s\n", endpoint);

    curl_easy_setopt(curl_handle, CURLOPT_URL, endpoint);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl_handle, CURLOPT_MAXREDIRS, 10);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        status = 1;
        goto end;
    }
    else
    {
        long response_code;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

        if (response_code != 200)
        {
            fprintf(stderr, "received HTTP status code %d\n", response_code);
            status = 1;
            goto end;
        }

        printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
        printf("%s\n", chunk.memory);
    }

    json_object *user_json = json_tokener_parse(chunk.memory);

    json_object *field;
    const char *value;

    if (!json_object_object_get_ex(user_json, "id", &field))
    {
        printf("id not found");
        status = 1;
        goto end;
    }

    value = json_object_get_string(field);
    user->id = malloc(sizeof(char) * strlen(value) + 1);
    strcpy(user->id, value);

    if (!json_object_object_get_ex(user_json, "nickname", &field))
    {
        printf("name not found");
        status = 1;
        goto end;
    }

    value = json_object_get_string(field);
    user->nickname = malloc(sizeof(char) * strlen(value) + 1);
    strcpy(user->nickname, value);

    if (!json_object_object_get_ex(user_json, "username", &field))
    {
        printf("username not found");
        status = 1;
        goto end;
    }

    value = json_object_get_string(field);
    user->username = malloc(sizeof(char) * strlen(value) + 1);
    strcpy(user->username, value);

end:

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    curl_slist_free_all(headers);

    free(tokenHeader);
    free(endpoint);

    json_object_put(user_json);
    free(chunk.memory);
    return status;
}

void mattermost_free_user(struct MatterMostUser *user)
{
    free(user->id);
    free(user->nickname);
    free(user->username);
    free(user);
}