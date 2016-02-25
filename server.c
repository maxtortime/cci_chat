#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "cci.h"

#define ACCEPT_CONTEXT (void*)0xfeebdaed
#define SEND_CONTEXT (void*)0xdaedfeeb

int main(int argc, char *argv[])
{
	int ret, done = 0;
	uint32_t caps = 0;
	char *uri = NULL;
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
			continue;
		}

        switch (event->type) {
            case CCI_EVENT_RECV:
                break;
            case CCI_EVENT_SEND:
                break;
            case CCI_EVENT_CONNECT_REQUEST:
                break;
            case CCI_EVENT_ACCEPT:
                break;
            default:
                break;
        }
    }

    /* clean up */
	cci_destroy_endpoint(endpoint);
	cci_finalize();
	free(uri);

	return 0;

}
