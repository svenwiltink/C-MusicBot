#include <libwebsockets.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mattermost.h"

static struct lws *web_socket = NULL;

static int mattermost_callback( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
		case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lws_callback_on_writable( web_socket );
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("RECEIVED DATA\n");
            printf("%.*s\n", len, (char *) in);
			/* Handle incomming messages here. */
			break;

		case LWS_CALLBACK_CLIENT_WRITEABLE:
		{
            printf("writeable bitches!\n");

            char *payload = "{ \"seq\": 1, \"action\": \"authentication_challenge\", \"data\": { \"token\": \"TOKEN\" } }";
			unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + (sizeof(char) * strlen(payload) + 1) + LWS_SEND_BUFFER_POST_PADDING];
			unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
			size_t n = sprintf( (char *)p, "%s", payload);
			lws_write( wsi, p, n, LWS_WRITE_TEXT );
			break;
		}

		case LWS_CALLBACK_CLOSED:
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf("we DEAD\n");
			web_socket = NULL;
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
		0,
		0,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

void mattermost_connect(struct MatterMostApiOptions options)
{
	struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;
    info.options=LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.timeout_secs = 10;

	struct lws_context *context = lws_create_context( &info );

	time_t old = 0;
	while( 1 )
	{
		struct timeval tv;
		gettimeofday( &tv, NULL );

		/* Connect if we are not connected to the server. */
		if( !web_socket && tv.tv_sec != old )
		{
            printf("creating a new connection!\n");
			struct lws_client_connect_info ccinfo = {0};
			ccinfo.context = context;
			ccinfo.address = "chat.transip.us";
			ccinfo.port = 443;
			ccinfo.path = "/api/v4/websocket";
			ccinfo.host = ccinfo.address;
			ccinfo.origin = ccinfo.address;
			ccinfo.protocol = protocols[0].name;
            ccinfo.ssl_connection = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4);
			web_socket = lws_client_connect_via_info(&ccinfo);
		}

		if( tv.tv_sec != old )
		{
			/* Send a random number to the server every second. */
			lws_callback_on_writable( web_socket );
			old = tv.tv_sec;
		}

		lws_service( context, /* timeout_ms = */ 250 );
	}

	lws_context_destroy( context );
}