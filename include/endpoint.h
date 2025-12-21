#ifndef ENDPOINT_H
#define ENDPOINT_H

// Linked list for internal use.
struct endpoint_node{
    char* ep;
    // Function pointer
    struct endpoint_node *next;
};

void http_endpoint_push(struct endpoint_node **head, char* ep);
void http_endpoint_delete(struct endpoint_node **head, char* ep); // do we need this?
// TODO return a function pointer.
struct endpoint_node* http_endpoint_get(char* ep); // Can do some fuzzy matching; input comes from the HTTP message.
void http_free_all_endpoints(struct endpoint_node **head);

#endif