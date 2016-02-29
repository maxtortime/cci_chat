#include "cci.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#define ACCEPT_CONTEXT (void*)0xfeebdaed
#define SEND_CONTEXT (void*)0xdaedfeeb
#define CONNECT_CONTEXT (void*)0xdeadbeef

#define MSG_SIZE 128

enum MODE { CLIENT, SERVER };

typedef struct {
    cci_os_handle_t *fd;
    cci_endpoint_t *endpoint;
    cci_connection_t *connection;
    cci_event_t *event;

    char* msg;

    int done; // 0 loop end , 1 keep
    enum MODE mode;
} cci_data;

void print_error(char *argv[], char* uri);

void poll_events(void *data);
void* input_msg(void* chat);

