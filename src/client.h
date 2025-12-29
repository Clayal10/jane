#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>

typedef struct {
    http_server *server;
    int fd;
} http_client;

void *http_handle_client(void *http_client);

#endif