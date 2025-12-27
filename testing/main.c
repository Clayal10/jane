#include<stdio.h>
#include "../src/parsing.h"
#include "../src/endpoint.h"

void example_handle_func(http_response_writer* w, http_request* req){
    printf("Hit endpoint!!\n");
}

void example_handle_func_two(http_response_writer* w, http_request* req){
    printf("Hit endpoint 2!!\n");
}

void unit_test_endpoints(){
    http_server* server = http_new_server(34567);
    http_handle_func(server, "/", &example_handle_func);
    http_handle_func(server, "/config/setup", &example_handle_func_two);
}

char* string_request = "GET /lurk-client/update/ HTTP/1.1\r\n"
"Host: localhost:5068\r\n"
"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:140.0) Gecko/20100101 Firefox/140.0\r\n"
"Accept: */*\r\n"
"Accept-Language: en-US,en;q=0.5\r\n"
"Accept-Encoding: gzip, deflate, br, zstd\r\n"
"Referer: http://localhost:5068/\r\n"
"Connection: keep-alive\r\n"
"Sec-Fetch-Dest: empty\r\n"
"Sec-Fetch-Mode: cors\r\n"
"Sec-Fetch-Site: same-origin\r\n"
"Priority: u=4\r\n"
"\r\n";

char* string_post_request = "POST /lurk-client/setup/ HTTP/1.1\r\n"
"Host: localhost:5068\r\n"
"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:140.0) Gecko/20100101 Firefox/140.0\r\n"
"Accept: application/json\r\n"
"Accept-Language: en-US,en;q=0.5\r\n"
"Accept-Encoding: gzip, deflate, br, zstd\r\n"
"Referer: http://localhost:5068/\r\n"
"Content-Type: application/json\r\n"
"Content-Length: 29\r\n"
"Origin: http://localhost:5068\r\n"
"Connection: keep-alive\r\n"
"Sec-Fetch-Dest: empty\r\n"
"Sec-Fetch-Mode: cors\r\n"
"Sec-Fetch-Site: same-origin\r\n"
"Priority: u=0\r\n"
"\r\n"
"{\"Hostname\":\"\",\"Port\":\"5069\"}";

void unit_test_parsing(){
    http_request_frame frame;
    decode_http(string_request, &frame);
    printf("Method: %d\nEndpoint: %s\nHost: %s\n", 
        frame.header->method, frame.header->endpoint, frame.header->host);
    free_http_header(frame.header);
    decode_http(string_post_request, &frame);
    printf("%d\n%s\n%s\n%d\n%s\n", 
        frame.header->method, frame.header->endpoint, frame.header->host, frame.header->content_length, frame.header->content_type);
}

int main(){
    unit_test_parsing();
    return 0;
}