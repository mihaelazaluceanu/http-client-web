#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "parson.h"
#include "requests.h"
#include "helpers.h"

#define HOST "34.246.184.49"
#define PORT 8080

int main() {
    char *message;
    char *response;
    char command[100];
    int cookies_count = 0;
    char **cookies = (char **)malloc(cookies_count * sizeof(char *));
    if (cookies == NULL) {
        perror("Malloc failed.");
        exit(1);
    }

    // se primesc comenzi cat timp nu se primeste comanda de exit
    while(1) {
        // se citeste comanda
        fgets(command, 100, stdin);
        command[strlen(command) - 1] = '\0';

        // se verifica daca comanda este exit
        if(strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "register") == 0) {
            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char username[100];
            char password[100];

            // se citesc username-ul si parola
            printf("username=");
            fgets(username, 100, stdin);
            username[strlen(username) - 1] = '\0';

            printf("password=");
            fgets(password, 100, stdin);
            password[strlen(password) - 1] = '\0';

            // se construieste obiectul JSON
            JSON_Value *main_obj = json_value_init_object();
            JSON_Object *reg_obj = json_value_get_object(main_obj);
            json_object_set_string(reg_obj, "username", username);
            json_object_set_string(reg_obj, "password", password);
            char *serialized_string = json_serialize_to_string_pretty(main_obj);

            // compute POST request
            message = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", &serialized_string, strlen(serialized_string), NULL, 0);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            free(serialized_string);
            json_value_free(main_obj);
            close_connection(sockfd);
            close(sockfd);

            // trebu de afisat mess de la server hz cum
        } else if (strcmp(command, "login") == 0) {
            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char username[100];
            char password[100];

            // se citesc username-ul si parola
            printf("username=");
            fgets(username, 100, stdin);
            username[strlen(username) - 1] = '\0';

            printf("password=");
            fgets(password, 100, stdin);
            password[strlen(password) - 1] = '\0';

            // se construieste obiectul JSON
            JSON_Value *main_obj = json_value_init_object();
            JSON_Object *reg_obj = json_value_get_object(main_obj);
            json_object_set_string(reg_obj, "username", username);
            json_object_set_string(reg_obj, "password", password);
            char *serialized_string = json_serialize_to_string_pretty(main_obj);

            // compute POST request
            message = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", &serialized_string, strlen(serialized_string), NULL, 0);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            free(serialized_string);
            json_value_free(main_obj);
            close_connection(sockfd);
            close(sockfd);

            // cumva se verifica daca operatia ii buna sau mi v gavne
        } else if (strcmp(command, "enter_library") == 0) {
            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // compute GET request
            message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookies, cookies_count);

            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori cumvata

            // se extrage raspunsul
            char *jwt = calloc(BUFLEN, sizeof(char));
            char *p = strstr(response, "{");
            char *q = strstr(p, "}");
            memcpy(jwt, p, q - p + 1);

            JSON_Value *value = json_parse_string(jwt);
            JSON_Object *obj = json_value_get_object(value);
            memset(jwt, 0, BUFLEN);
            char *token = (char *)json_object_get_string(obj, "token");
            strcpy(jwt, token);

            puts("200 - OK - Entered library.");

            json_value_free(value);
            free(jwt);
            close_connection(sockfd);
            close(sockfd);
        } else if (strcmp(command, "get_books") == 0) {
            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // compute GET request
            message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, cookies, cookies_count);

            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori cumvata

            // se extrage raspunsul
            char *books = strstr(response, "[");
            JSON_Value *main_obj = json_parse_string(books);
            JSON_Array *books_arr = json_value_get_array(main_obj);

            for (int i = 0; i < json_array_get_count(books_arr); i++) {
                JSON_Object *book_obj = json_array_get_object(books_arr, i);
                char *id = (char *)json_object_get_string(book_obj, "id");
                char *title = (char *)json_object_get_string(book_obj, "title");
                char *author = (char *)json_object_get_string(book_obj, "author");
                char *genre = (char *)json_object_get_string(book_obj, "genre");
                char *publisher = (char *)json_object_get_string(book_obj, "publisher");
                char *page_count = (char *)json_object_get_string(book_obj, "page_count");
                printf("id=%s, title=%s, author=%s, genre=%s, publisher=%s, page_count=%s\n", id, title, author, genre, publisher, page_count);
            }
        } else if (strcmp(command, "get_book") == 0) {
            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        } else if (strcmp(command, "add_book") == 0) {
            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        } else if (strcmp(command, "delete_book") == 0) {
            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
        } else {
            printf("Invalid command.\n");
        }
    }

    free(cookies);
    free(message);
    free(response);
    return 0;
}
