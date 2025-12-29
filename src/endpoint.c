#include<stdlib.h>
#include<string.h>

#include "endpoint.h"

static void free_endpoint_elements(endpoint_node* node);

void http_endpoint_push(endpoint_node **head, char* ep, void(*func)(http_response_writer*,http_request*)){
    endpoint_node* new = malloc(sizeof(endpoint_node));
    int len = strlen(ep);
    new->ep = malloc(len+1);
    memcpy(new->ep, ep, len);
    new->ep[len] = 0;
    new->func = func;
    new->next = NULL;
    
    if(!*head){
        *head = new;
        return;
    }
    endpoint_node* current = *head;
    for(;current->next;){
        current = current->next;
    }
    current->next = new;
}

endpoint_node* http_endpoint_get(endpoint_node** head, char* ep){
    if(!*head){
        return NULL;
    }
    endpoint_node* current = *head;
    for(;current;){
        if(endpoint_cmp(current->ep, ep)){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void http_free_all_endpoints(endpoint_node** head){
    if(!*head){
        return;
    }
    endpoint_node *current, *next;
    current = *head;
    for(;current;){
        next = current->next;
        free_endpoint_elements(current);
        free(current);
        current = next;
    }
}

static void free_endpoint_elements(endpoint_node* node){
    if(node->ep){
        free(node->ep);
        node->ep = NULL;
    }
    if(node->func){
        free(node->func);
        node->func = NULL;
    }
    // freeing node->next is taken care of elsewhere.
}

int endpoint_cmp(char* ep, char* req){
    if(!ep || !req){
        return 0;
    }
    int ep_len = strlen(ep);
    if(ep_len != strlen(req) || ep_len == 0){
        return 0;
    }
    for(int i=0; i<ep_len; i++){
        if(ep[i] != req[i]){
            return 0;
        }
    }
    return 1;
}