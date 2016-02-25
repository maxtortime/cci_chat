#include "cci.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

int iters = 10;
int send_done = 0;
int recv_done = 0;
int flags = 0;
/* By default the connection is reliable ordered; users can change the
   connection type via the command line */
cci_conn_attribute_t attr = CCI_CONN_ATTR_RO;

#define CONNECT_CONTEXT (void*)0xdeadbeef


int main(int argc, char *argv[])
{
	int done = 0, ret, i = 0, c;
	uint32_t caps = 0;
	char *server_uri = NULL;
	cci_os_handle_t *fd = NULL;
	cci_endpoint_t *endpoint = NULL;
	cci_connection_t *connection = NULL;
	uint32_t timeout = 30 * 1000000;

	while ((c = getopt(argc, argv, "h:c:b")) != -1) {
		switch (c) {
		case 'h':
			server_uri = strdup(optarg);
			break;
		case 'c':
			if (strncasecmp ("ru", optarg, 2) == 0)
				attr = CCI_CONN_ATTR_RU;
			else if (strncasecmp ("ro", optarg, 2) == 0)
				attr = CCI_CONN_ATTR_RO;
			else if (strncasecmp ("uu", optarg, 2) == 0)
				attr = CCI_CONN_ATTR_UU;
			break;
		case 'b':
			flags |= CCI_FLAG_BLOCKING;
			break;
		default:
			fprintf(stderr, "usage: %s -h <server_uri> [-c <type>]\n",
			        argv[0]);
			fprintf(stderr, "\t-c\tConnection type (UU, RU, or RO) "
			                "set by client; RO by default\n");
			exit(EXIT_FAILURE);
		}
	}

	if (!server_uri) {
		fprintf(stderr, "usage: %s -h <server_uri> [-c <type>]\n", argv[0]);
		fprintf(stderr, "\t-c\tConnection type (UU, RU, or RO) "
                                        "set by client; RO by default\n");
		exit(EXIT_FAILURE);
	}

	ret = cci_init(CCI_ABI_VERSION, 0, &caps);
	if (ret) {
		fprintf(stderr, "cci_init() failed with %s\n",
			cci_strerror(NULL, ret));
		exit(EXIT_FAILURE);
	}

	/* create an endpoint */
	ret = cci_create_endpoint(NULL, 0, &endpoint, fd);
	if (ret) {
		fprintf(stderr, "cci_create_endpoint() failed with %s\n",
			cci_strerror(NULL, ret));
		exit(EXIT_FAILURE);
	}

	/* set conn tx timeout */
	cci_set_opt(endpoint, CCI_OPT_ENDPT_SEND_TIMEOUT,
		    &timeout);
	if (ret) {
		fprintf(stderr, "cci_set_opt() failed with %s\n",
			cci_strerror(endpoint, ret));
		exit(EXIT_FAILURE);
	}

	/* initiate connect */
	ret =
	    cci_connect(endpoint, server_uri, "Hello World!", 12,
			attr, CONNECT_CONTEXT, 0, NULL);
	if (ret) {
		fprintf(stderr, "cci_connect() failed with %s\n",
			cci_strerror(endpoint, ret));
		exit(EXIT_FAILURE);
	}

    
    /* polling events */
    while(!done) {
        cci_event_t * event;

        ret = cci_get_event(endpoint, &event);
		if (ret != 0) {
			if (ret != CCI_EAGAIN)
				fprintf(stderr, "cci_get_event() returned %s\n",
					cci_strerror(endpoint, ret));
			continue;
		}
        

        switch (event->type) {
            case CCI_EVENT_RECV:
                break;
            case CCI_EVENT_SEND:
                break;
            case CCI_EVENT_CONNECT:
                *done = 1;

                assert(event->connect.connection != NULL);
                assert(event->connect.connection->context == CONNECT_CONTEXT);

                *connection = event->connect.connection;

                break;
            default:
                fprintf(stderr,"ignoring event type %d\n", event->type);
                break;
        }
        cci_return_event(event);
    }

	ret = cci_finalize();
	if (ret) {
		fprintf(stderr, "cci_finalize() failed with %s\n",
			cci_strerror(NULL, ret));
		exit(EXIT_FAILURE);
	}

	return 0;

}

