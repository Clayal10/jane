#include <stdio.h>

struct http_client{
    struct http_server *server;
    int fd;
};

void *http_handle_client(void *http_client);
