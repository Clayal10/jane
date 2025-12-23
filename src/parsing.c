#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parsing.h"

char* decode_http(char* buffer, http_request_frame *frame){
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
            break;
        }
    }
    /* Potentially parse body */
}

void decode_http_header(http_request_header_frame *header, char* buffer, size_t len){
    /* First ASCII line */
    int method_end = 0;
    for(int i=0; i<len-1; i++){
        if(buffer[i] == ASCII_CR && buffer[i+1] == ASCII_LF){
            method_end = i;
            break;
        }
    }
    if(!method_end){
        perror("No valid ascii lines found in the HTTP buffer");
        return;
    }

    char method[method_end+1];
    memcpy(method, buffer, method_end);
    method[method_end] = 0;
    int offset;

    if(method[0] == ASCII_G){
        header->method = GET;
        offset = 4;
    }else{
        // Other methods.
        return;
    }
    int endpoint_end;
    for(endpoint_end=offset; endpoint_end<method_end; endpoint_end++){
        if(method[endpoint_end] == ASCII_SPACE){
            break;
        }
    }
    header->endpoint = malloc(endpoint_end-offset+1);
    memcpy(header->endpoint, &method[offset], endpoint_end-offset);
    header->endpoint[endpoint_end-offset] = 0;
    /* Need to parse host and content-type at least*/
}

void free_http_header(http_request_header_frame *header){
    if(!header){
        return;
    }
    if(header->endpoint){
        free(header->endpoint);
    }
    if(header->endpoint){
        free(header->host);
    }
    if(header->endpoint){
        free(header->content_type);
    }
    free(header);
}
