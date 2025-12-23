#ifndef ENDPOINT_H
#define ENDPOINT_H

#include "../include/http.h"

// Linked list for internal use.
typedef struct endpoint_node{
    char* ep;
    void(*func)(http_response_writer*, http_request*);
    struct endpoint_node *next;
} endpoint_node;

void http_endpoint_push(endpoint_node **head, endpoint_node* new);
void http_endpoint_delete(endpoint_node **head, char* ep); // do we need this?
void (*http_endpoint_get(http_response_writer*, http_request*)) (char* ep); // Can do some fuzzy matching; input comes from the HTTP message.
void http_free_all_endpoints(endpoint_node **head);

#endif