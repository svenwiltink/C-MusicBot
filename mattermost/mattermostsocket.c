#include <libwebsockets.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mattermost.h"

static int mattermost_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	struct MatterMostSession *session = (MatterMostSession *)user;

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:;
		struct MatterMostEvent event = {(char *)in};

		printf("WE GOT DATA");
		if (session->eventhandler != NULL)
		{
			session->eventhandler(session, event);
		}
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
	{
		struct MatterMostSession *sessionData = (struct MatterMostSession *)user;
		printf(sessionData->name);
		printf("writeable bitches!\n");

		// TODO: MOVE and replace with apioptions
		char *payload = "{ \"seq\": 1, \"action\": \"authentication_challenge\", \"data\": { \"token\": \"TOKEN\" } }";
		unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + (sizeof(char) * strlen(payload) + 1) + LWS_SEND_BUFFER_POST_PADDING];
		unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
		size_t n = sprintf((char *)p, "%s", payload);
		lws_write(wsi, p, n, LWS_WRITE_TEXT);
		break;
	}

	case LWS_CALLBACK_CLOSED:
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		printf("we DEAD\n");
		session->lws_websocket = NULL;
		break;

	default:
		printf("got event %d\n", reason);
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

void mattermost_session_destroy(struct MatterMostSession *session) {
	if (session->lws_context) {
		lws_context_destroy(session->lws_context);
	}

	session->lws_websocket = NULL;
}