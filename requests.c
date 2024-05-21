#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3 (optional): add headers and/or cookies, according to the protocol format
    sprintf(line, "User-Agent: Mozilla/5.0");
    compute_message(message, line);

    sprintf(line, "Connection: keep-alive");
    compute_message(message, line);

    if (cookies != NULL) {
        sprintf(line, "Cookie: ");

        for(int i = 0; i < cookies_count - 1; i++){
            sprintf(line, "%s%s; ", line, cookies[i]);
        }

        sprintf(line, "%s%s", line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    // Step 4: add final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // Step 1: write the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // Step 2: add the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Step 3: add necessary headers (Content-Type and Content-Length are mandatory)
    sprintf(line, "Connection: keep-alive");
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    int len = 0;
    for(int i = 0; i < body_data_fields_count - 1; i++){
        len += strlen(body_data[i]) + 1;
    }

    len += strlen(body_data[body_data_fields_count - 1]);

    sprintf(line, "Content-Length: %d", len);
    compute_message(message, line);
    
    // Step 4 (optional): add cookies
    if (cookies != NULL) {
       sprintf(line, "Cookie: ");

        for(int i = 0; i < cookies_count - 1; i++){
            sprintf(line, "%s%s; ", line, cookies[i]);
        }

        sprintf(line, "%s%s", line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    // Step 5: add new line at end of header
    compute_message(message, "");

    // Step 6: add the actual payload data
    for(int i = 0; i < body_data_fields_count - 1; i++){
        sprintf(body_data_buffer, "%s%s&", body_data_buffer, body_data[i]);
    }

    sprintf(body_data_buffer, "%s%s", body_data_buffer, body_data[body_data_fields_count - 1]);

    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}
