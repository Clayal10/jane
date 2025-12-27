#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parsing.h"

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
            return i;
        }
    }
    // TODO parse payload
    return -1;
}

// We know that 'buffer' contains all necessary bytes for the header if called from decode_http
void decode_http_header(http_request_header_frame *header, char* buffer, size_t len){
    /* First ASCII line */
    int method_end = 0;
    for(;method_end<len-1; method_end++){
        if(buffer[method_end] == ASCII_CR && buffer[method_end+1] == ASCII_LF){
            break;
        }
    }

    char method[method_end+1];
    memcpy(method, buffer, method_end);
    method[method_end] = 0;
    int offset;
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

    /* Need to parse host, content-length and content-type at least*/
    // Skip 'HTTP/1.1 to get to 'Host'
    offset = endpoint_end;
    for(;offset < len-2;offset++){
        if(buffer[offset] == ASCII_CR && buffer[offset+1] == ASCII_LF){
            offset += 2;
            break;
        }
    }
    int host_end = offset;
    for(;host_end < len-2; host_end++){
        if(buffer[host_end] == ASCII_CR && buffer[host_end+1] == ASCII_LF){
            break;
        }
    }
    
    offset += 6; // Skip 'HOST: '
    header->host = malloc(host_end-offset+1);
    memcpy(header->host, &buffer[offset], host_end-offset);
    header->host[host_end-offset] = 0;
    offset += host_end-offset;

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
            decode_post_request(header, buffer, len, offset);
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

    
}

void decode_post_request(http_request_header_frame *header, char* buffer, size_t len, int offset){
    // Need to get to 'Content-type'
     offset += 2; // skip crlf
     const char* content_type = "Content-Type: ";
     int content_type_len = strlen(content_type);
     for(;offset<len-content_type_len;offset++){
         if(memcmp(&buffer[offset], content_type, content_type_len) == 0){
             break;
         }
     }
    // Used for POST
    offset += content_type_len; // skip 'Content-Type: '
    int content_type_end = offset;
    for(;content_type_end < len-2;content_type_end++){
        if(buffer[content_type_end] == ASCII_CR && buffer[content_type_end] == ASCII_LF){
            break;
        }
    }
    // If these fields didn't appear for whatever reason, this section would just allocate 1 
    // byte for a lone null terminator.
    header->content_type = malloc(content_type_end-offset+1);
    memcpy(header->content_type, &buffer[offset], content_type_end-offset);
    header->content_type[content_type_end-offset] = 0;
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
