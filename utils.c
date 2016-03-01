#include "cci-chat.h"

void* input_msg(void *chat)
{
    int ret = 0;
    char* msg = calloc(MSG_SIZE,sizeof(char));

    cci_data* data = (cci_data*) chat;

    ret = cci_get_event(data->endpoint, &(data->event));

    cci_os_handle_t *fd = data->fd;
    cci_endpoint_t *endpoint = data->endpoint;
    cci_connection_t *connection = data->connection;

    printf("> ");
    scanf("%s",msg);
    
    ret = cci_send(connection, msg, MSG_SIZE, SEND_CONTEXT, 0);

    if (ret)
        fprintf(stderr, "send returned %s\n",
                cci_strerror(endpoint, ret));
                
    free(msg);

    pthread_exit(NULL);
}

void print_error(char* argv[], char* uri)
{
    if (!uri) {
        fprintf(stderr, "usage: %s -h <server_uri> [-c <type>]\n", argv[0]);
        fprintf(stderr, "\t-c\tConnection type (UU, RU, or RO) "
                "set by client; RO by default\n");
        exit(EXIT_FAILURE);
    }
}

void poll_events(void *data) {
    int ret = 0;
    void * void_p = NULL;
    cci_data* chat = (cci_data*) data;

event: ret = cci_get_event(chat->endpoint, &(chat->event));
    
    if (ret != 0) {
        if (ret != CCI_EAGAIN)
            fprintf(stderr, "cci_get_event() returned %s\n", cci_strerror(chat->endpoint, ret));
        
        goto event;
    }

    switch (chat->event->type) {
        case CCI_EVENT_RECV:
            assert(chat->event->recv.connection == chat->connection);
            /* server's context : ACCEPT 
             * client's context : CONNECT
             */
            assert(chat->event->recv.connection -> context == (chat->mode ? ACCEPT_CONTEXT : CONNECT_CONTEXT));

            fprintf(stderr,"%s\n",(char *) chat->event->recv.ptr);
            break;
            /* client do noting when sending  */
        case CCI_EVENT_SEND:
            if (chat->mode == SERVER) {
                assert(chat->event->send.context == SEND_CONTEXT);
                assert(chat->event->send.connection == chat->connection);
                assert(chat->event->send.connection->context == ACCEPT_CONTEXT);
            }
            break;
            /* for server */
        case CCI_EVENT_CONNECT_REQUEST:
            cci_accept(chat->event, ACCEPT_CONTEXT);
            break;
        case CCI_EVENT_ACCEPT:
            assert(chat->event->accept.connection != NULL);
            assert(chat->event->accept.connection->context == ACCEPT_CONTEXT);

            if(!chat->connection) chat->done = 1;

            chat->connection = chat->event->accept.connection;
            break;
            /* for client */
        case CCI_EVENT_CONNECT:
            assert(chat->event->connect.connection != NULL);
            assert(chat->event->connect.connection->context == CONNECT_CONTEXT);

            if(!chat->connection) chat->done = 1;

            chat->connection = chat->event->connect.connection;
            break;
        default:
            fprintf(stderr,"ignoring event type %d\n",chat->event->type);
            break;
    }
    cci_return_event(chat->event);
}
