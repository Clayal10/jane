#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parsing.h"

const char* field_host = "Host: ";
const char* field_content_type = "Content-Type: ";
const char* field_content_length = "Content-Length: ";
const char* field_accept = "Accept: ";

int decode_http(char* buffer, http_request_frame *frame){
    if(!frame){
        perror("HTTP request frame cannot be null");
        return 0;
    }
    size_t buffer_len = strlen(buffer);
    frame->header = malloc(sizeof(http_request_header_frame));
    int i;
    for(i=0; i<buffer_len-3; i++){
        if(buffer[i] == ASCII_CR && buffer[i+1] == ASCII_LF && buffer[i+2] == ASCII_CR && buffer[i+3] == ASCII_LF){
            decode_http_header(frame->header, buffer, i);
            return i+4+frame->header->content_length;
        }
    }
    // TODO parse payload
    return -1;
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
        header->method = CONNECT;
        break;
    case ASCII_D:
        header->method = DELETE;
        break;
    case ASCII_G:
        header->method = GET;
        break;
    case ASCII_H:
        header->method = HEAD;
        break;
    case ASCII_O:
        header->method = OPTIONS;
        break;
    case ASCII_P:
        if(method[1] == ASCII_O){
            header->method = POST;
        }else if(method[1] == ASCII_A){
            header->method = PATCH;
        }else{
            header->method = PUT;
        }
        break;
    case ASCII_T:
        header->method = TRACE;
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

void free_http_header(http_request_header_frame *header){
    if(!header){
        return;
    }
    if(header->endpoint){
        free(header->endpoint);
    }
    if(header->host){
        free(header->host);
    }
    if(header->content_type){
        free(header->content_type);
    }
    free(header);
}
