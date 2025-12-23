#ifndef HTTP_H
#define HTTP_H

/*
This header file contains objects and functions that are in the public API
for this library.
*/

/*
Priority for an HTTP server:
    1. Serving HTML pages
    2. Serving static directory for css/js
    3. Registering endpoints
    4. Serving TLS
*/

#include <stdint.h>

typedef struct http_server http_server;
// http_request contains information regarding the request sent by the client.
typedef struct http_request{} http_request;
typedef struct http_response_writer{
    int client_fd;
    // Other function pointers.
    void (*write)(void*);
    void (*write_header)(void*);
} http_response_writer;

// The new HTTP server is allocated on the heap and must be freed by calling http_free_server(server);
http_server *http_new_server(char* hostname, uint16_t port);
void http_free_server(http_server *server);
// http_listen_and_serve will start the http server that will execute functions on given endpoints from
// the function http_handle_func.
int http_listen_and_serve(http_server *server);
// When 'endpoint' is hit by a client, 'func' will be ran. Parameters for the function
// contain an http writer that you need for writing and reading via HTTP as well as the request
// given by the client.
void http_handle_func(http_server *server, char* endpoint, void(*func)(http_response_writer*, http_request*));

#endif