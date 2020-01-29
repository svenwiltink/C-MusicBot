#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libwebsockets.h>
#include <json-c/json.h>

#include "mattermost.h"

struct MatterMostSession
{
	struct MatterMostApiOptions apiOptions;
	MatterMostHandleEvent eventhandler;
	enum MatterMostSessionStates state;
	struct lws_context *lws_context;
	struct lws *lws_websocket;
	time_t lastPing;
};

void mattermost_parse_channel(struct MatterMostChannel *channel, json_object *event_json)
{
	json_object *displayNameField;
	const char *displayNameValue;

	json_object_object_get_ex(event_json, "channel_display_name", &displayNameField);
	displayNameValue = json_object_get_string(displayNameField);

	channel->displayname = malloc(sizeof(char) * strlen(displayNameValue) + 1);
	strcpy(channel->displayname, displayNameValue);
};

void mattermost_free_channel(struct MatterMostChannel channel)
{
	free(channel.displayname);
}

static int mattermost_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	struct MatterMostSession *session = (struct MatterMostSession *)user;

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		session->state = MATTERMOST_SESSION_AUTHENTICATING;

		// set callback so we can send our initial authentication message
		lws_callback_on_writable(session->lws_websocket);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
	{
		json_object *event_json = json_tokener_parse((char *)in);

		json_object *eventTypeField;
		const char *eventTypeName;

		/**
		 * We can either get a status response or an event.
		 */
		if (json_object_object_get_ex(event_json, "status", &eventTypeField))
		{
			eventTypeName = json_object_get_string(eventTypeField);

			json_object *seqField;
			int32_t seqValue;

			json_object_object_get_ex(event_json, "seq_reply", &seqField);
			seqValue = json_object_get_int(seqField);

			printf("got status event %s. status %s seq: %d\n", (char *)in, eventTypeName, seqValue);

			if (seqValue != 1)
			{
				printf("ignoring status for seq %d", seqValue);
				goto end;
			}

			if (strcmp(eventTypeName, "OK") != 0)
			{
				printf("authentication failed: %s\n", (char *)in);
				session->state = MATTERMOST_SESSION_AUTHENTICATION_FAILED;
			}

			session->state = MATTERMOST_SESSION_CONNECTED;

			goto end;
		}

		if (!json_object_object_get_ex(event_json, "event", &eventTypeField))
		{
			printf("got unknown reponse %s\n", (char *)in);
			goto end;
		}

		eventTypeName = json_object_get_string(eventTypeField);
		printf("got event type %s\n", eventTypeName);

		if (session->eventhandler == NULL)
		{
			printf("no eventhandler defined\n");
			goto end;
		}

		json_object *eventData;
		json_object_object_get_ex(event_json, "data", &eventData);

		if (strcmp(eventTypeName, "posted") == 0)
		{
			struct MatterMostEventPosted *event = malloc(sizeof(struct MatterMostEventPosted));
			mattermost_parse_channel(&event->channel, eventData);

			session->eventhandler(session, MATTERMOST_EVENT_TYPE_POSTED, event);

			mattermost_free_channel(event->channel);
			free(event);
		}
		else
		{
			printf("unsupported event %s\n", eventTypeName);
		}

	end:

		json_object_put(event_json);
		break;
	}
	case LWS_CALLBACK_CLIENT_WRITEABLE:
	{
		if (session->state == MATTERMOST_SESSION_AUTHENTICATING)
		{
			session->state = MATTERMOST_SESSION_AUTHENTICATING_WAITING;
			size_t needed = snprintf(NULL, 0, "{ \"seq\": 1, \"action\": \"authentication_challenge\", \"data\": { \"token\": \"%s\" } }", session->apiOptions.token) + 1;
			char *payload = malloc(needed);
			sprintf(payload, "{ \"seq\": 1, \"action\": \"authentication_challenge\", \"data\": { \"token\": \"%s\" } }", session->apiOptions.token);

			unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + (sizeof(char) * strlen(payload) + 1) + LWS_SEND_BUFFER_POST_PADDING];
			unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
			size_t n = sprintf((char *)p, "%s", payload);
			lws_write(wsi, p, n, LWS_WRITE_TEXT);

			free(payload);
		}
		break;
	}

	case LWS_CALLBACK_CLOSED:
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		printf("we DEAD\n");
		session->state = MATTERMOST_SESSION_DISCONNECTED;
		break;

	default:
		break;
	}

	return 0;
}

static struct lws_protocols protocols[] =
	{
		{
			"mattermost-protocol",
			mattermost_callback,
			sizeof(struct MatterMostSession),
			0,
		},
		{NULL, NULL, 0, 0} /* terminator */
};

struct MatterMostSession *mattermost_init(struct MatterMostApiOptions apiOptions)
{
	struct MatterMostSession *session = calloc(1, sizeof(struct MatterMostSession));
	session->apiOptions = apiOptions;
	return session;
}

int mattermost_get_state(struct MatterMostSession *session)
{
	return session->state;
}

void mattermost_set_eventhandler(struct MatterMostSession *session, MatterMostHandleEvent eventHandler)
{
	session->eventhandler = eventHandler;
}

void mattermost_connect(struct MatterMostSession *session, struct MatterMostApiOptions options)
{
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof(info));

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
	info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	info.timeout_secs = 10;

	session->lws_context = lws_create_context(&info);

	printf("creating a new connection!\n");

	struct lws_client_connect_info ccinfo = {0};
	ccinfo.context = session->lws_context;
	ccinfo.address = session->apiOptions.endpoint;
	ccinfo.port = 443;
	ccinfo.path = "/api/v4/websocket";
	ccinfo.host = ccinfo.address;
	ccinfo.origin = ccinfo.address;
	ccinfo.protocol = protocols[0].name;
	ccinfo.userdata = session;
	ccinfo.ssl_connection = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);

	session->lws_websocket = lws_client_connect_via_info(&ccinfo);
}

void mattermost_service(struct MatterMostSession *session)
{
	lws_service(session->lws_context, 250);

	time_t newTime;
	time(&newTime);

	if (newTime - session->lastPing > 10)
	{
		printf("we should ping now\n");
		lws_callback_on_writable(session->lws_websocket);
		time(&session->lastPing);
	}
}

void mattermost_session_free(struct MatterMostSession *session)
{
	if (session->lws_context)
	{
		lws_context_destroy(session->lws_context);
	}

	session->lws_websocket = NULL;
	free(session);
}