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
#include "parsing.h"

struct http_server{
    uint16_t port;
    endpoint_node *head;
    pthread_mutex_t mu;
};

http_server* http_new_server(uint16_t port){
    http_server *server = (http_server*)malloc(sizeof(http_server));
    pthread_mutex_init(&server->mu, NULL);
    server->port = htons(port);
    server->head = NULL;
    return server;
}

void http_free_server(http_server *server){
    if(!server){
        perror("Error: 'server' is null");
        return;
    }
    http_free_all_endpoints(&server->head);
    pthread_mutex_destroy(&server->mu);
    free(server);
}

int http_listen_and_serve(http_server *server){
    if(!server){
        perror("Error: 'server' cannot be null");
        return 0;
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
        if(client_fd == -1){
            printf("Error: %s\n", strerror(errno));
            continue;
        }
        // Read from each client that comes in, wait for HTTP request.
        // If we receive an HTTP request:
        //  - Check if the endpoint is registered in the server.
        //  - Run the function associated with the endpoint; pass the client_fd into the function.
        http_client client;
        client.server = server;
        client.fd = client_fd;

        pthread_t thread_handle;
        pthread_create(&thread_handle, 0, &http_handle_client, (void*)&client);
    }
}

void *http_handle_client(void *c){
    http_client *client = (http_client*)c;
    if(!client->server){
        return 0;
    }
    size_t buffer_len = 1024*64; // 64 kb should be enough.
    char *buffer = malloc(buffer_len);
    int read_offset = 0;
    for(;;){
        size_t count = read(client->fd, &buffer[read_offset], buffer_len);
        if(!count){
            break;
        }
        if(count == -1){
            break;
        }
        
        int bytes_processed = 0;
        for(;;){
            http_request_frame frame;
            // status is either -1 to indicate the need for more data or the offset of the 
            // buffer the message ended at.
            int status = decode_http(&buffer[bytes_processed], &frame, (count+read_offset)-bytes_processed);
            if(status == -1){
                // If we need more data, add to the buffer and reprocess the whole thing.
                // This applies if the payload is sent in a separate message from the header.
                read_offset = count;
                goto error;
            }
            bytes_processed += status;
            // we read and parsed a valid message, now we need to continue processing other messages
            // in the buffer or read again to get a new message.
            read_offset = 0;

            endpoint_node *ep = http_endpoint_get(&client->server->head, frame.header->endpoint);
            if(!ep){
                char* err_message = malloc(512);
                sprintf(err_message, "Endpoint %s not registered.\n", frame.header->endpoint);
                perror(err_message);
                free(err_message);
                goto error;
            }

            printf("Calling function\n");
            ep->func(0, 0);

            free_http_fields(&frame);
            if(bytes_processed = count){
                break;
            }
            continue;

        error:
            free_http_fields(&frame);
            break;
        }
        // 1. Parse HTTP request
        // 2. Get the function corresponding to the endpoint.
        // 3. Create an http_request with the proper header and body previously parsed.
        // 4. Give the function an instance of an http_response_writer that can write back to
        //    this client_fd.
        // Example:
        // endpoint_node* ep = http_endpoint_get(&client->server->head, frame.header->endpoint);
        // ep->func(0, 0);
    }
    close(client->fd);
    free(buffer);
}

void http_handle_func(http_server *server, char* endpoint, void(*func)(http_response_writer*, http_request*)){
    http_endpoint_push(&server->head, endpoint, func);    
}
