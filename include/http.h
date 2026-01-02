#ifndef HTTP_H
#define HTTP_H

/*
This header file contains objects and functions that are in the public API
for this library.
*/

#include <stdint.h>

typedef struct http_server http_server;

// http_request contains information regarding the request sent by the client.
// TODO: Look into what else should go here.
typedef struct {
    // String method name. EX: "GET", "POST", etc.
    char* method;
    // Raw string of the body content. Is likely JSON, can create a JSON parser.
    char* body;
} http_request;

typedef struct {
    int fd;
} http_response_writer;

typedef enum{
    METHOD_INVALID, // for the 0 value.
    METHOD_CONNECT,
    METHOD_DELETE,
    METHOD_GET,
    METHOD_HEAD,
    METHOD_OPTIONS,
    METHOD_PATCH,
    METHOD_POST,
    METHOD_PUT,
    METHOD_TRACE,
} http_method;

typedef enum{
    STATUS_OK = 200,
    STATUS_NO_CONTENT = 204,
    STATUS_BAD_REQUEST = 400,
    STATUS_FORBIDDEN = 403,
    STATUS_NOT_FOUND = 404,
    STATUS_INTERNAL_SERVER_ERROR = 500,
    STATUS_NOT_IMPLEMENTED = 501,
} http_status_code;

// The new HTTP server is allocated on the heap and must be freed by calling http_free_server(server);
// 'port' will be converted to big endian.
// TODO: add support for architectures that aren't little endian.
http_server *http_new_server(uint16_t port);
// Freeing the HTTP server also frees all of the endpoints created with http_handle_func.
void http_free_server(http_server *server);
// http_listen_and_serve will start the http server that will execute functions on given endpoints from
// the function http_handle_func.
int http_listen_and_serve(http_server *server);
// When 'endpoint' is hit by a client, 'func' will be ran. Parameters for the function
// contain an http writer that you need for writing and reading via HTTP as well as the request
// given by the client.
void http_handle_func(http_server *server, char* endpoint, void(*func)(http_response_writer*, http_request*));
// http_write will write data in buffer to the socket via HTTP. 'buffer' should be the desired payload. A status
// code of 200 will be written when this function is successfully called.
void http_write(http_response_writer* w, char* buffer);
// http_writer_header will write an http status code via HTTP.
void http_write_header(http_response_writer* w, int status_code);

#endif