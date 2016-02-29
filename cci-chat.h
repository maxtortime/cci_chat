#include "cci.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define ACCEPT_CONTEXT (void*)0xfeebdaed
#define SEND_CONTEXT (void*)0xdaedfeeb
#define CONNECT_CONTEXT (void*)0xdeadbeef

#define MSG_SIZE 128
