#include "cci-chat.h"

int flags = 0;
cci_conn_attribute_t attr = CCI_CONN_ATTR_RO;

int main(int argc, char *argv[])
{
    int done = 0; // if done = 1, loop end
    int ret = 0; // return variable for cci function
    uint32_t caps = 0; // Don't know how to use 
    char *server_uri = NULL;
    int c = 0;

    int msg_thr_id; /* id of msg input thread */
    pthread_t m_thread;

    enum MODE mode = SERVER;
    
    cci_data* chat;

    chat = malloc(sizeof(cci_data));
    
    chat->fd = NULL;
    chat->endpoint = NULL;
    chat->connection = NULL;

    uint32_t timeout = 30 * 1000000;

    if (argc > 1) // if argc is 2, server mode
    {
        mode = CLIENT;

        // Configure program option 
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
                    print_error(argv, server_uri);
            }
        }
    } 

    if (mode == CLIENT && !server_uri) 
        print_error(argv, server_uri);

    /* both server and client do */
    /* initialize CCI */
    ret = cci_init(CCI_ABI_VERSION, 0, &caps);
    if (ret) {
        fprintf(stderr, "cci_init() failed with %s\n",
                cci_strerror(NULL, ret));
        exit(EXIT_FAILURE);
    }

    /* create an endpoint */
    ret = cci_create_endpoint(NULL, 0, &(chat->endpoint), chat->fd);
    if (ret) {
        fprintf(stderr, "cci_create_chat->endpoint() failed with %s\n",
                cci_strerror(NULL, ret));
        exit(EXIT_FAILURE);
    }

    if (mode == SERVER) 
    { 
        /* get option */
        ret = cci_get_opt(chat->endpoint,
                CCI_OPT_ENDPT_URI, &server_uri);
        if (ret) {
            fprintf(stderr, "cci_get_opt() failed with %s\n", cci_strerror(NULL, ret));
            exit(EXIT_FAILURE);
        }
        printf("Opened %s\n", server_uri);
    }
    else if (mode == CLIENT) 
    {
        /* set conn tx timeout */
        cci_set_opt(chat->endpoint, CCI_OPT_ENDPT_SEND_TIMEOUT,
                &timeout);
        if (ret) {
            fprintf(stderr, "cci_set_opt() failed with %s\n",
                    cci_strerror(chat->endpoint, ret));
            exit(EXIT_FAILURE);
        }

        /* initiate connect */
        ret =
            cci_connect(chat->endpoint, server_uri, "Hello World!", 12,
                    attr, CONNECT_CONTEXT, 0, NULL);
        if (ret) {
            fprintf(stderr, "cci_connect() failed with %s\n",
                    cci_strerror(chat->endpoint, ret));
            exit(EXIT_FAILURE);
        }
    }

    while(!done) {
        cci_event_t *event;

        ret = cci_get_event(chat->endpoint, &event);

        if (ret != 0) {
            if (ret != CCI_EAGAIN)
                fprintf(stderr, "cci_get_event() returned %s\n",
                        cci_strerror(chat->endpoint, ret));
            else if(chat->connection) 
                pthread_create(&m_thread, NULL, input_msg, (void*) chat);
            
            continue;
        }

        switch (event->type) {
            case CCI_EVENT_RECV:
                assert(event->recv.connection == chat->connection);
                /* server's context : ACCEPT 
                 * client's context : CONNECT
                 */
                assert(event->recv.connection -> context == (mode ? ACCEPT_CONTEXT : CONNECT_CONTEXT));

                fprintf(stderr,"%s\n",(char *) event->recv.ptr);
                break;
                /* client do noting when sending  */
            case CCI_EVENT_SEND:
                if (mode == SERVER) {
                    assert(event->send.context == SEND_CONTEXT);
                    assert(event->send.connection == chat->connection);
                    assert(event->send.connection->context == ACCEPT_CONTEXT);
                }
                break;
                /* for server */
            case CCI_EVENT_CONNECT_REQUEST:
                cci_accept(event, ACCEPT_CONTEXT);
                break;
            case CCI_EVENT_ACCEPT:
                assert(event->accept.connection != NULL);
                assert(event->accept.connection->context == ACCEPT_CONTEXT);
                chat->connection = event->accept.connection;
                break;
                /* for client */
            case CCI_EVENT_CONNECT:
                assert(event->connect.connection != NULL);
                assert(event->connect.connection->context == CONNECT_CONTEXT);

                chat->connection = event->connect.connection;
                break;
            default:
                fprintf(stderr,"ignoring event type %d\n",event->type);
                break;
        }
        cci_return_event(event);
    }
    /* clean up */
    free(server_uri);
    free(chat);

    cci_destroy_endpoint(chat->endpoint);
    cci_finalize();

    return 0;
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

    /* Remove trailing newline, . */
    if ((strlen(msg)>0) && (msg[strlen(msg) - 1] == '\n')) 
        msg[strlen(msg) - 1] = '\0';
    
    ret = cci_send(connection, msg, MSG_SIZE, SEND_CONTEXT, 0);

    if (ret)
        fprintf(stderr, "send returned %s\n",
                cci_strerror(endpoint, ret));

    free(msg);
    pthread_exit(NULL);
}
