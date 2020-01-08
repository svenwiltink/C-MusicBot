#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

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

    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

    /* check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    else
    {
        printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
        printf("%s\n", chunk.memory);
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
    curl_slist_free_all(headers);

    free(tokenHeader);
    free(endpoint);

    cJSON *user_json = cJSON_Parse(chunk.memory);

    if (user_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        goto end;
    }

    cJSON *field;


    field = cJSON_GetObjectItemCaseSensitive(user_json, "id");
    if (!cJSON_IsString(field) || field->valuestring == NULL)
    {
        printf("id not found");
        goto end;
    }

    user->id = malloc(sizeof(char) * strlen(field->valuestring) + 1);
    strcpy(user->id, field->valuestring);


    field = cJSON_GetObjectItemCaseSensitive(user_json, "username");
    if (!cJSON_IsString(field) || field->valuestring == NULL)
    {
        printf("username not found");
        goto end;
    }

    user->username = malloc(sizeof(char) * strlen(field->valuestring) + 1);
    strcpy(user->username, field->valuestring);

    field = cJSON_GetObjectItemCaseSensitive(user_json, "nickname");
    if (!cJSON_IsString(field) || field->valuestring == NULL)
    {
        printf("nickname not found");
        goto end;
    }

    user->nickname = malloc(sizeof(char) * strlen(field->valuestring) + 1);
    strcpy(user->nickname, field->valuestring);

end:
    cJSON_Delete(user_json);
    free(chunk.memory);
}