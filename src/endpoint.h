#ifndef ENDPOINT_H
#define ENDPOINT_H

#include "../include/http.h"

// Figure out how to return a function pointer :/
#define HANDLE_FUNC void(*)(http_response_writer*,http_request*)

// Linked list for internal use.
typedef struct endpoint_node{
    char* ep;
    void(*func)(http_response_writer*, http_request*);
    struct endpoint_node *next;
} endpoint_node;

void http_endpoint_push(endpoint_node **head, char* ep, void(*func)(http_response_writer*,http_request*));
// Return value needs to be casted to a HANDLE_FUNC for now.
endpoint_node* http_endpoint_get (endpoint_node** head, char* ep); // Can do some fuzzy matching; input comes from the HTTP message.
// endpoints have to be exactly the same for now.
int endpoint_cmp(char* ep, char* req);
void http_free_all_endpoints(endpoint_node **head);

#endif