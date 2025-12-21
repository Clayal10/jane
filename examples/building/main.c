#include<jane/http.h>

int main(){
    http_server *server = http_new_server("0.0.0.0", htons(5000));

    http_listen_and_serve(server);

    http_free_server(server);
    return 0;
}