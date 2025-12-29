#include<jane/http.h>
#include<stdio.h>
#include<stdlib.h>

void func_to_be_handled(http_response_writer*,http_request* req){
    printf("%s", req->method);
}

int main(int argc, char** argv){
    if(argc != 2){
        return -1;
    }
    http_server *server = http_new_server(atoi(argv[1]));

    http_handle_func(server, "/", &func_to_be_handled);
    http_listen_and_serve(server);

    http_free_server(server);
    return 0;
}