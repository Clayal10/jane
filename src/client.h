#include <stdio.h>

typedef struct http_client{
    struct http_server *server;
    int fd;
} http_client;

void *http_handle_client(void *http_client);
