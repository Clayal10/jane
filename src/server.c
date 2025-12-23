#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/http.h"
#include "client.h"
#include "endpoint.h"

struct http_server{
    char* hostname;
    uint16_t port;
    endpoint_node **head;
    pthread_mutex_t mu;
};

http_server* http_new_server(char* hostname, uint16_t port){
    http_server *server = (http_server*)malloc(sizeof(http_server));
    server->hostname = malloc(strlen(hostname)+1);
    strcpy(server->hostname, hostname);
    pthread_mutex_init(&server->mu, NULL);
    server->port = port;
    return server;
}

void http_free_server(http_server *server){
    if(!server){
        perror("Error: 'server' is null");
        return;
    }
    free(server->hostname);
    pthread_mutex_destroy(&server->mu);
    free(server);
}

int http_listen_and_serve(http_server *server){
    if(!server){
        perror("Error: 'server' cannot be null");
        return;
    }

    struct sockaddr_in socket_address;
    socket_address.sin_port = server->port;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_family = AF_INET;

    int skt = socket(AF_INET, SOCK_STREAM, 0);
    if(skt == -1){
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    if(bind(skt, (struct sockaddr*)(&socket_address), sizeof(struct sockaddr_in))){
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    if(listen(skt, 5)){
        printf("Error: %s\n", strerror(errno));
        return -1;
    }

    // As we accept connections, each connection will run in a thread and process HTTP requests
    // for that client. Each thread will keep track of their client_fd.
    for(;;){
        struct sockaddr_in client_address;
		socklen_t address_size = sizeof(struct sockaddr_in);
		int client_fd = accept(skt, (struct sockaddr *)(&client_address), &address_size);
        // Read from each client that comes in, wait for HTTP request.
        // If we receive an HTTP request:
        //  - Check if the endpoint is registered in the server.
        //  - Run the function associated with the endpoint; pass the client_fd into the function.
        struct http_client client;
        client.server = server;
        client.fd = client_fd;

        pthread_t thread_handle;
        pthread_create(&thread_handle, 0, &http_handle_client, (void*)&client);
    }
}

void *http_handle_client(void *c){
    http_client *client = (http_client*)c;
    if(!client->server){
        return;
    }
    size_t buffer_len = 2048;
    char *buffer = malloc(buffer_len);
    for(;;){
        size_t count = read(client->fd, buffer, buffer_len);
        if(!count){
            break;
        }
        if(count == -1){
            printf("Error: %s\n", strerror(errno));
            break;
        }
        // 1. Parse HTTP request
        // 2. Get the function corresponding to the endpoint.
        // 3. Create an http_request with the proper header and body previously parsed.
        // 4. Give the function an instance of an http_response_writer that can write back to
        //    this client_fd.
    }
    free(buffer);
}

void http_handle_func(http_server *server, char* endpoint, void(*func)(http_response_writer*, http_request*)){
    // Create an endpoint node and push it on the list. 
}
