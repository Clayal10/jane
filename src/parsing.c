#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parsing.h"

const char* field_host = "Host: ";
const char* field_content_type = "Content-Type: ";
const char* field_content_length = "Content-Length: ";
const char* field_accept = "Accept: ";

const char* status_ok_text = "HTTP/1.1 200 OK\r\n";
const char* status_no_content_text = "HTTP/1.1 204 No Content\r\n";
const char* status_bad_request_text = "HTTP/1.1 400 Bad Request\r\n";
const char* status_forbidden_text = "HTTP/1.1 403 Forbidden\r\n";
const char* status_not_found_text = "HTTP/1.1 404 Not Found\r\n";
const char* status_internal_server_error_text = "HTTP/1.1 500 Internal Server Error\r\n";
const char* status_not_implemented_text = "HTTP/1.1 501 Not Implemented\r\n";

static char* get_status_code(http_status_code status);

int decode_http_request(char* buffer, http_request_frame *frame, size_t buffer_len){
    // buffer_len indicates the number of bytes that were read into the buffer.
    frame->header = malloc(sizeof(http_request_header_frame));
    frame->header->method = METHOD_INVALID;
    frame->header->content_type = NULL;
    frame->header->content_length = 0;
    frame->header->endpoint = NULL;
    frame->header->host = NULL;
    frame->body = NULL;
    
    int i;
    for(i=0; i<buffer_len-3; i++){
        if(buffer[i] == ASCII_CR && buffer[i+1] == ASCII_LF && buffer[i+2] == ASCII_CR && buffer[i+3] == ASCII_LF){
            decode_http_header(frame->header, buffer, i);
            i += 4; // Skip to start of the payload
            goto success;
        }
    }

    // need more data
    return -1;

success:

    int offset = i;
    if(!frame->header->content_length){
        return offset;
    }

    if(buffer_len < offset+frame->header->content_length){
        // need more data for the rest of the body
        return -1;
    }
    
    frame->body = malloc(frame->header->content_length+1);
    memcpy(frame->body, &buffer[offset], frame->header->content_length);
    frame->body[frame->header->content_length] = 0;
    return offset + frame->header->content_length;
}

// We know that 'buffer' contains all necessary bytes for the header if called from decode_http
void decode_http_header(http_request_header_frame *header, char* buffer, size_t len){
    int method_end = 0;
    for(;method_end<len-1; method_end++){
        if(buffer[method_end] == ASCII_CR && buffer[method_end+1] == ASCII_LF){
            break;
        }
    }

    char method[method_end+1];
    memcpy(method, buffer, method_end);
    method[method_end] = 0;
    int offset = 0;
    for(;offset<method_end; offset++){
        if(method[offset] == ASCII_SPACE){
            offset++;
            break;
        }
    }

    int endpoint_end = offset;
    for(; endpoint_end<method_end; endpoint_end++){
        if(method[endpoint_end] == ASCII_SPACE){
            break;
        }
    }
    header->endpoint = malloc(endpoint_end-offset+1);
    memcpy(header->endpoint, &method[offset], endpoint_end-offset);
    header->endpoint[endpoint_end-offset] = 0;

    switch(method[0]){
    case ASCII_C:
        header->method = METHOD_CONNECT;
        break;
    case ASCII_D:
        header->method = METHOD_DELETE;
        break;
    case ASCII_G:
        header->method = METHOD_GET;
        break;
    case ASCII_H:
        header->method = METHOD_HEAD;
        break;
    case ASCII_O:
        header->method = METHOD_OPTIONS;
        break;
    case ASCII_P:
        if(method[1] == ASCII_O){
            header->method = METHOD_POST;
        }else if(method[1] == ASCII_A){
            header->method = METHOD_PATCH;
        }else{
            header->method = METHOD_PUT;
        }
        break;
    case ASCII_T:
        header->method = METHOD_TRACE;
        break;
    }

    /* Need to parse host, content-length and content-type at least */
    // Skip 'HTTP/1.1'
    
    size_t field_host_len = strlen(field_host);
    size_t field_content_type_len = strlen(field_content_type);
    size_t field_content_length_len = strlen(field_content_length);

    offset = method_end + 2; // puts us at the start of the first field after endpoint.
    for(;offset<len;){
        int end = offset;
        for(;;end++){
            if(buffer[end] == ASCII_CR && buffer[end+1] == ASCII_LF){
                break;
            }
        }
        // offset is at the start of the string, end is the index of '\r'.
        if(!memcmp(&buffer[offset], field_host, field_host_len)){
            offset += field_host_len;
            header->host = malloc(end-offset+1);
            memcpy(header->host, &buffer[offset], end-offset);
            header->host[end-offset] = 0;
        }
        else if(!memcmp(&buffer[offset], field_content_type, field_content_type_len)){
            offset += field_content_type_len;
            header->content_type = malloc(end-offset+1);
            memcpy(header->content_type, &buffer[offset], end-offset);
            header->content_type[end-offset] = 0;
        }
        else if(!memcmp(&buffer[offset], field_content_length, field_content_length_len)){
            offset += field_content_length_len;
            char content_length[end-offset+1];
            memcpy(content_length, &buffer[offset], end-offset);
            content_length[end-offset] = 0;
            header->content_length = atoi(content_length);
        }

        offset = end + 2;
    }
}

void free_http_fields(http_request_frame* frame){
    if(!frame){
        return;
    }
    if(frame->header->endpoint){
        free(frame->header->endpoint);
        frame->header->endpoint = NULL;
    }
    if(frame->header->host){
        free(frame->header->host);
        frame->header->host = NULL;
    }
    if(frame->header->content_type){
        free(frame->header->content_type);
        frame->header->content_type = NULL;
    }
    if(frame->header){
        free(frame->header);
        frame->header = NULL;
    }
    if(frame->body){
        free(frame->body);
        frame->body = NULL;
    }
}

// Invalid frame fields will cause this function to return NULL. If not NULL,
// you have the responsibility of freeing the response data.
char* encode_http_response(http_response_frame *frame){
    char* data = frame->header->content_length + 1024*8; // reserve max 8 KB for header
    size_t status_code_length = 0;
    switch(frame->header->status_code){
    case STATUS_OK:
        status_code_length = strlen(status_ok_text);
        memcpy(data, status_ok_text, status_code_length);
        break;
    case STATUS_NO_CONTENT:
        status_code_length = strlen(status_no_content_text);
        memcpy(data, status_no_content_text, status_code_length);
        break;
    case STATUS_BAD_REQUEST:
        status_code_length = strlen(status_bad_request_text);
        memcpy(data, status_bad_request_text, status_code_length);
        break;
    case STATUS_FORBIDDEN:
        status_code_length = strlen(status_forbidden_text);
        memcpy(data, status_forbidden_text, status_code_length);
        break;
    case STATUS_NOT_FOUND:
        status_code_length = strlen(status_not_found_text);
        memcpy(data, status_not_found_text, status_code_length);
        break;
    case STATUS_INTERNAL_SERVER_ERROR:
        status_code_length = strlen(status_internal_server_error_text);
        memcpy(data, status_internal_server_error_text, status_code_length);
        break;
    case STATUS_NOT_IMPLEMENTED:
        status_code_length = strlen(status_not_implemented_text);
        memcpy(data, status_not_implemented_text, status_code_length);
        break;
    default:
        return NULL;
    }
    int offset = status_code_length;

    
    memcpy(&data[offset], "\r\n", 2);
    offset += 2;
    char* return_data = malloc(offset);
    memcpy(return_data, data, offset);
    free(data);
    return return_data;
}
