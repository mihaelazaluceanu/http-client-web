#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *jwt)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // writes the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // adds the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // adds headers and/or cookies, according to the protocol format
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

    if (jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    // adds final new line
    compute_message(message, "");
    return message;
}

char *compute_del_request(char *host, char *url, char *query_params,
                            char **cookies, int cookies_count, char *jwt)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // writes the method name, URL, request params (if any) and protocol type
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    // adds the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // adds headers and/or cookies, according to the protocol format
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

    if (jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    // adds final new line
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                            int body_data_fields_count, char **cookies, int cookies_count, char *jwt)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    // writes the method name, URL and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    // adds the host
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // adds necessary headers (Content-Type and Content-Length are mandatory)
    sprintf(line, "Connection: keep-alive");
    compute_message(message, line);

    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    sprintf(line, "Content-Length: %ld", strlen(*body_data));
    compute_message(message, line);
    
    // adds cookies
    if (cookies != NULL) {
       sprintf(line, "Cookie: ");

        for(int i = 0; i < cookies_count - 1; i++){
            sprintf(line, "%s%s; ", line, cookies[i]);
        }

        sprintf(line, "%s%s", line, cookies[cookies_count - 1]);
        compute_message(message, line);
    }

    if (jwt != NULL) {
        sprintf(line, "Authorization: Bearer %s", jwt);
        compute_message(message, line);
    }

    // adds new line at end of header
    compute_message(message, "");

    // adds the actual payload data
    compute_message(message, *body_data);

    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    return message;
}
