#include<jane/http.h>
#include<stdio.h>

void func_to_be_handled(http_response_writer*,http_request*){
    printf("Hello!!!");
    // these types aren't actually supported yet.
}

int main(){
    http_server *server = http_new_server(5000);

    http_handle_func(server, "/", &func_to_be_handled);
    http_listen_and_serve(server);

    http_free_server(server);
    return 0;
}