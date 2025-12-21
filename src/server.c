#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "../include/http.h"
#include "client.h"
#include "endpoint.h"

struct http_server{
    char* hostname;
    uint16_t port;
    endpoint_node **head;
};

http_server* http_new_server(char* hostname, uint16_t port){
    http_server *server = (http_server*)malloc(sizeof(http_server));
    server->hostname = malloc(strlen(hostname)+1);
    strcpy(server->hostname, hostname);
    server->port = port;
    return server;
}

void http_free_server(http_server *server){
    if(!server){
        return;
    }
    free(server->hostname);
    free(server);
}

int http_listen_and_serve(http_server *server){
    if(!server){
        perror("Error: 'server' cannot be null");
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
    for(;;){
        // read http request
        write(client->fd, "Hello\n", 6); // for now.
        break;
    }
}