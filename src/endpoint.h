#ifndef ENDPOINT_H
#define ENDPOINT_H

// Linked list for internal use.
typedef struct endpoint_node{
    char* ep;
    // Function pointer
    struct endpoint_node *next;
} endpoint_node;

void http_endpoint_push(endpoint_node **head, char* ep);
void http_endpoint_delete(endpoint_node **head, char* ep); // do we need this?
// TODO return a function pointer.
endpoint_node* http_endpoint_get(char* ep); // Can do some fuzzy matching; input comes from the HTTP message.
void http_free_all_endpoints(endpoint_node **head);

#endif