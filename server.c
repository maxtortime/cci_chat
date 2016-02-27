#include "cci_chat.h"

int main(int argc, char *argv[])
{
	int ret, done = 0;
	uint32_t caps = 0;
	char *uri = NULL;
    char* msg;
    msg = calloc(MSG_SIZE, sizeof(char));

	cci_endpoint_t *endpoint = NULL;
	cci_os_handle_t *ep_fd = NULL;
	cci_connection_t *connection = NULL;

	ret = cci_init(CCI_ABI_VERSION, 0, &caps);
	if (ret) {
		fprintf(stderr, "cci_init() failed with %s\n",
			cci_strerror(NULL, ret));
		exit(EXIT_FAILURE);
	}

	/* create an endpoint */
	ret = cci_create_endpoint(NULL, 0, &endpoint, ep_fd);
	if (ret) {
		fprintf(stderr, "cci_create_endpoint() failed with %s\n",
			cci_strerror(NULL, ret));
		exit(EXIT_FAILURE);
	}

	ret = cci_get_opt(endpoint,
			  CCI_OPT_ENDPT_URI, &uri);
	if (ret) {
		fprintf(stderr, "cci_get_opt() failed with %s\n", cci_strerror(NULL, ret));
		exit(EXIT_FAILURE);
	}
	printf("Opened %s\n", uri);

    
    while(!done) {
        cci_event_t * event;

        ret = cci_get_event(endpoint, &event);
		if (ret != 0) {
			if (ret != CCI_EAGAIN)
				fprintf(stderr, "cci_get_event() returned %s\n",
					cci_strerror(endpoint, ret));
            else {
                if (connection) {
                    printf("> ");
                    fgets(msg, MSG_SIZE, stdin); 
                    ret = cci_send(connection, msg, MSG_SIZE, SEND_CONTEXT, 0);

                    if (ret)
                        fprintf(stderr, "send returned %s\n",
                            cci_strerror(endpoint, ret));
                }
            }
			continue;
		}

        switch (event->type) {
            case CCI_EVENT_RECV: {
                    assert(event->recv.connection == connection);
                    assert(event->recv.connection-> context == ACCEPT_CONTEXT);
                    
                    fprintf(stderr, "%s\n", (char *) event->recv.ptr);
                    
                    break;
                }
            case CCI_EVENT_SEND:
                assert(event->send.context == SEND_CONTEXT);
                assert(event->send.connection == connection);
                assert(event->send.connection->context == ACCEPT_CONTEXT);
                break;
            case CCI_EVENT_CONNECT_REQUEST:
                cci_accept(event, ACCEPT_CONTEXT);
                break;
            case CCI_EVENT_ACCEPT:
                assert(event->accept.connection != NULL);
                assert(event->accept.connection->context == ACCEPT_CONTEXT);

                connection = event->accept.connection;

                break;
            default:
                printf("event type %d\n",event->type);
                break;
        }
    }

    /* clean up */
	cci_destroy_endpoint(endpoint);
	cci_finalize();
	free(uri);
    free(msg);

	return 0;
}
