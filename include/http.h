#ifndef HTTP_H
#define HTTP_H

/*
Priority for an HTTP server:
    1. Serving HTML pages
    2. Serving static directory for css/js
    3. Registering endpoints
    4. Serving TLS
*/

#include<stdint.h>

struct http_server{
    char* hostname;
    uint16_t port;
};

struct http_server new_http_server();
void listen_and_serve(struct http_server);

#endif