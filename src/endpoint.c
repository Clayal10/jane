#include<stdlib.h>
#include<string.h>

#include "endpoint.h"

void http_endpoint_push(endpoint_node **head, char* ep, void(*func)(http_response_writer*,http_request*)){
    endpoint_node* new = malloc(sizeof(endpoint_node));
    int len = strlen(ep);
    new->ep = malloc(len+1);
    memcpy(new->ep, ep, len);
    new->func = func;
    
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

endpoint_node* http_endpoint_get (endpoint_node** head, char* ep){
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
    for(;;){
        next = current->next;
        free(current);
        if(!next){
            break;
        }
        current = next;
    }
}

int endpoint_cmp(char* ep, char* req){
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