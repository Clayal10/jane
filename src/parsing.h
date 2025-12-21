#define CR 13
#define NL 10

typedef enum{
    GET,
    POST
} Method;

// Fields only include the fields required by the server to complete a response.
typedef struct http_request_header_frame{
    Method method;
    char* endpoint;
    char* host;
    char* content_type;
    int content_length;
}http_request_header_frame;

typedef struct http_request_frame{
    http_request_header_frame header;
    char* body;
}http_request_frame;

typedef struct http_response_header_frame{

} http_response_header_frame;

typedef struct http_respones_frame{
    http_response_header_frame header;
}http_response_frame;

// Parses 'buf' for an http request. If there is not enough data for a header, NULL will be returned. 
// If a header frame can be parsed, the corrsponding information will be removed from 'buf' to be 
// returned and used for future processing to prevent message loss. A whole request frame is given, but
// it is likely that only the header is used if there is no body in the request.
char* decode_http(char* buf, http_request_frame *frame);
// Prepares response for a buffer of bytes to send over http back to the client.
char* encode_http(http_response_frame response);