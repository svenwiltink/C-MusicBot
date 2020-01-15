#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <libwebsockets.h>
#include <json-c/json.h>

#include "mattermost.h"

static int mattermost_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	struct MatterMostSession *session = (MatterMostSession *)user;

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		session->state = MATTERMOST_SESSION_AUTHENTICATING;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
	{
		struct MatterMostEvent event = {(char *)in};

		json_object *event_json = json_tokener_parse(event.data);

		json_object *field;
		const char *fieldValue;

		/**
		 * We can either get a status response or an event.
		 */
		if (json_object_object_get_ex(event_json, "status", &field))
		{
			fieldValue = json_object_get_string(field);

			json_object *seqField;
			int32_t seqValue;

			json_object_object_get_ex(event_json, "seq_reply", &seqField);
			seqValue = json_object_get_int(seqField);

			printf("got status event %s. status %s seq: %d\n", event.data, fieldValue, seqValue);

			if (seqValue != 1)
			{
				printf("ignoring status for seq %d", seqValue);
				goto end;
			}

			if (strcmp(fieldValue, "OK") != 0)
			{
				printf("authentication failed: %s\n", event.data);
				session->state = MATTERMOST_SESSION_AUTHENTICATION_FAILED;
			}

			session->state = MATTERMOST_SESSION_CONNECTED;

			goto end;
		}

		if (!json_object_object_get_ex(event_json, "event", &field))
		{
			printf("got unknown reponse %s\n", event.data);
			goto end;
		}

		fieldValue = json_object_get_string(field);
		printf("got event type %s\n", fieldValue);

		if (session->eventhandler != NULL)
		{
			session->eventhandler(session, event);
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

void mattermost_session_free(struct MatterMostSession *session)
{
	if (session->lws_context)
	{
		lws_context_destroy(session->lws_context);
	}

	session->lws_websocket = NULL;
}