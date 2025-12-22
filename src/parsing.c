#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parsing.h"

static void decode_http_header(http_request_header_frame *header, char* buffer, size_t len);

char* decode_http(char* buffer, http_request_frame *frame){
    if(!frame){
        perror("HTTP request frame cannot be null");
        return 0;
    }
    size_t buffer_len = strlen(buffer);
    http_request_header_frame header;
    int i;
    for(i=0; i<buffer_len-3; i++){
        if((int)buffer[i] == CR && (int)buffer[i+1] == NL && (int)buffer[i+2] == CR && (int)buffer[i+3] == NL){
            printf("here\n");
            decode_http_header(&header, buffer, i);
            break;
        }
    }
}

static void decode_http_header(http_request_header_frame *header, char* buffer, size_t len){
    int i;
    for(i=0; i<len-1; i++){
        if((int)buffer[i] == CR && (int)buffer[i+1] == NL){
            goto success;
        }
    }
    printf("No lines.\n");
    return;
success:
    
    // Could be off by one?
    char method[i];
    strncpy(method, buffer, i-1);
    method[i-1] = 0;
    if((int)method[0] == G){
        header->method = GET;
        int j;
        for(j=4; method+j; j++){
            if((int)method[j] == SPACE){
                break;
            }
        }
        char endpoint[j-4];
        strncpy(endpoint, &method[4], j-5);
        endpoint[j-5] = 0;
        header->endpoint = endpoint;
    }else{
        return;
    }

    if(len < i+2){
        return;
    }
    buffer = buffer + (i+2); // should put us beyond the \n and we are supposed to have more lines.
}


