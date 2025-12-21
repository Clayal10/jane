#ifndef HTTP_H
#define HTTP_H

/*
Priority for an HTTP server:
    1. Serving HTML pages
    2. Serving static directory for css/js
    3. Registering endpoints
    4. Serving TLS
This is ideally the only public header file. Currently the Makefile makes all
headers in this directory public.
*/

#include <stdint.h>
#include "endpoint.h"
#include "client.h"

struct http_server{
    char* hostname;
    uint16_t port;
    struct endpoint_node **head;
};

// http_request contains information regarding the request sent by the client.
struct http_request{};
// http_response_writer contains functions that can be used to write a response back to the client.
// This writer is used to abstract the HTTP protocol from the user.
struct http_response_writer{
    int client_fd;
    // Other function pointers.
};



// Fields should be populated and port should be in network order.
int http_listen_and_serve(struct http_server *server);
// When 'endpoint' is hit by a client, 'func' will be ran. Parameters for the function
// contain an http writer that you need for writing and reading via HTTP as well as the request
// given by the user.
void http_handle_func(struct http_server *server, char* endpoint, void(func)(struct http_response_writer*, struct http_request*));

#endif