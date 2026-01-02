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
#include <time.h>
#include <locale.h>
#include "../include/http.h"
#include "client.h"
#include "endpoint.h"
#include "parsing.h"

struct http_server{
    endpoint_node *head;
    uint16_t port;
    pthread_mutex_t mu;
};

static void populate_http_request(http_request* req, http_request_frame frame);
static void free_http_request(http_request* req);
static void new_response_writer(http_response_writer* w);
static void free_response_writer(http_response_writer* w);

http_server* http_new_server(uint16_t port){
    http_server *server = malloc(sizeof(http_server));
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
    server->head = NULL;
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
        if(client_fd < 0){
            printf("Error: %s\n", strerror(errno));
            continue;
        }
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

    // Create http_response_writer with the socket.
    http_response_writer writer;
    writer.fd = client->fd;

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
            int status = decode_http_request(&buffer[bytes_processed], &frame, (count+read_offset)-bytes_processed);
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

            // Each different client points to the same server.
            pthread_mutex_lock(&client->server->mu);
            endpoint_node *ep = http_endpoint_get(&client->server->head, frame.header->endpoint);
            pthread_mutex_unlock(&client->server->mu);
            if(!ep){
                char* err_message = malloc(512);
                sprintf(err_message, "Endpoint %s not registered.\n", frame.header->endpoint);
                perror(err_message);
                free(err_message);
                goto error;
            }

            http_request req;
            populate_http_request(&req, frame);
            ep->func(&writer, &req);

            free_http_request(&req);
            free_http_fields(&frame);
            if(bytes_processed = count){
                break;
            }
            continue;

        error:
            free_http_fields(&frame);
            break;
        }
    }
    close(client->fd);
    free(buffer);
}

void http_handle_func(http_server *server, char* endpoint, void(*func)(http_response_writer*, http_request*)){
    http_endpoint_push(&server->head, endpoint, func);    
}

// All objects and buffers will be allocated and freed within these functions.
void http_write(http_response_writer* w, char* buffer){
    // 1. Create http_response_frame and populate it with the relevant information.
    // 2. Write it to the socket.
}

void http_write_header(http_response_writer* w, http_status_code status_code){
    // Essentially the same as http_write
    http_response_frame *resp = malloc(sizeof(http_response_frame));
    http_response_header_frame *header = malloc(sizeof(http_response_header_frame));
    header->status_code = status_code;
    header->content_length = 0;
    header->content_type = NULL;
    char buffer[128];
    time_t t;
    time(&t);
    struct tm *gm_time = gmtime(&t);
    setlocale(LC_ALL, "C");
    size_t n = strftime(buffer, sizeof(buffer), "%a, %d %m %Y %H:%M:%S GMT", gm_time);
    header->date = malloc(n+1);
    memcpy(header->date, buffer, n);
    header->date[n] = 0;
    resp->header = header;
    char* data = encode_http_response(resp);
    // Write the data
    free(resp->header->date);
    free(resp->header);
    free(resp);
}

static void populate_http_request(http_request* req, http_request_frame frame){
    req->method = NULL;
    req->body = NULL;
    
    char method[10];
    memset(method, 0, 10); // to always ensure a null terminator.
    switch(frame.header->method){
    case METHOD_CONNECT:
        strcpy(method, "CONNECT");
        break;        
    case METHOD_DELETE:
        strcpy(method, "DELETE");
        break;
    case METHOD_GET:
        strcpy(method, "GET");
        break;
    case METHOD_HEAD:
        strcpy(method, "HEAD");
        break;
    case METHOD_OPTIONS:
        strcpy(method, "OPTIONS");
        break;
    case METHOD_PATCH:
        strcpy(method, "PATCH");
        break;
    case METHOD_POST:
        strcpy(method, "POST");
        break;
    case METHOD_PUT:
        strcpy(method, "PUT");
        break;
    case METHOD_TRACE:
        strcpy(method, "TRACE");
        break;
    default:
        return;
    }
    size_t len = strlen(method);
    req->method = malloc(len+1);
    memcpy(req->method, method, len);
    req->method[len] = 0;

    if(frame.body){
        len = strlen(frame.body);
        req->body = malloc(len+1);
        memcpy(req->body, frame.body, len);
        req->body[len] = 0;
    }
}

static void free_http_request(http_request* req){
    if(req->body){
        free(req->body);
        req->body = NULL;
    }
    if(req->method){
        free(req->method);
        req->method = NULL;
    }
}

static void new_response_writer(http_response_writer* w){
    
}
