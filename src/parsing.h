#define LINE_END_SEQ "\r\n";
#define MSG_END_SEQ "\r\n\r\n";

// Only a couple methods for now.
typedef enum{
    GET,
    POST,
    PUT,
    DELETE
} Method;

// Need to know the basics of what is being sent. The server will send
// any type, but I'm only expecting on receiving application/json or application/octet-stream for now.
typedef enum{
    JSON,
    OCTET,
    TEXT
} Content_type;

// I'm just going to discard a lot of information for now.
typedef struct http_request_header_frame{
    Method method;
    char* endpoint;
    Content_type content_type;
}http_header_frame;
typedef struct http_request_frame{}http_request_frame;

// Parses 'buf' for a header frame. If there is not enough data for a header, NULL will be
// returned and 'buf' will remain unchanged. If a header frame can be parsed, the corrsponding
// information will be removed from 'buf' to be used again.
http_request_header_frame decode_header(char* buf);