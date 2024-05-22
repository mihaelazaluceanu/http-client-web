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
    // array pentru stocarea cookie-urilor sesiunii curente
    char **cookies = (char **)malloc(cookies_count * sizeof(char *));
    if (cookies == NULL) {
        perror("Malloc failed.");
        exit(1);
    }

    // variabila pentru a stoca token-ul JWT
    char *jwt = calloc(BUFLEN, sizeof(char));
    if (jwt == NULL) {
        perror("Calloc failed.");
        exit(1);
    }

    // se primesc comenzi pana la intalnirea comenzii 'exit'
    while(1) {
        // se citeste comanda de la tastatura
        fgets(command, 100, stdin);
        command[strlen(command) - 1] = '\0';

        // se verifica daca comanda este 'exit'
        if(strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "register") == 0) {
            // se verifica daca este deja logat un user
            if (cookies_count != 0) {
                puts("ERROR - User already logged in.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char username[100];
            char password[100];

            // se citeste username-ul
            printf("username=");
            fgets(username, 100, stdin);
            username[strlen(username) - 1] = '\0';
            
            // se citeste parola
            printf("password=");
            fgets(password, 100, stdin);
            password[strlen(password) - 1] = '\0';

            // se construieste obiectul JSON
            JSON_Value *main_obj = json_value_init_object();
            JSON_Object *reg_obj = json_value_get_object(main_obj);
            json_object_set_string(reg_obj, "username", username);
            json_object_set_string(reg_obj, "password", password);
            char *ser_data = json_serialize_to_string(main_obj);

            // compute POST request
            message = compute_post_request(HOST, "/api/v1/tema/auth/register", "application/json", &ser_data, strlen(ser_data), NULL, 0, NULL);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica validitatea raspunsului
            if (strstr(response, "400 Bad Request") != NULL){
                puts("ERROR - Username already exists.");
            } else {
                puts("SUCCESS - User registered successfully.");
            }

            free(ser_data);
            json_value_free(main_obj);
            close_connection(sockfd);
            close(sockfd);
        } else if (strcmp(command, "login") == 0) {
            // se verifica daca este deja logat un user
            if (cookies_count != 0) {
                puts("ERROR - User already logged in.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
            char username[100];
            char password[100];

            // se citesc username-ul
            printf("username=");
            fgets(username, 100, stdin);
            username[strlen(username) - 1] = '\0';
            
            // se citeste parola
            printf("password=");
            fgets(password, 100, stdin);
            password[strlen(password) - 1] = '\0';

            // se construieste obiectul JSON
            JSON_Value *main_obj = json_value_init_object();
            JSON_Object *log_obj = json_value_get_object(main_obj);
            json_object_set_string(log_obj, "username", username);
            json_object_set_string(log_obj, "password", password);
            char *ser_data = json_serialize_to_string(main_obj);

            // compute POST request
            message = compute_post_request(HOST, "/api/v1/tema/auth/login", "application/json", &ser_data, strlen(ser_data), NULL, 0, NULL);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica validitatea raspunsului
            if (strstr(response, "400 Bad Request") != NULL) {
                puts("ERROR - Invalid username or password.");
            } else {
                puts("SUCCESS - User logged in successfully.");

                // se extrage cookie-ul
                char *line = strtok(response, "\n");
                while (line) {
                    if (strncmp(line, "Set-Cookie: ", 12) == 0) {
                        char *cookie_start = line + 12;
                        char *cookie_end = strchr(cookie_start, ';');
                        cookies[cookies_count] = strndup(cookie_start, cookie_end - cookie_start);
                        cookies_count++;
                        break;
                    }

                    line = strtok(NULL, "\n");
                }
            }

            free(ser_data);
            json_value_free(main_obj);
            close_connection(sockfd);
            close(sockfd); 
        } else if (strcmp(command, "enter_library") == 0) {
            // se verifica daca user-ul este logat
            if (cookies_count == 0) {
                puts("ERROR - User not logged in.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // compute GET request
            message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, cookies, cookies_count, jwt);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori
            if (strstr(response, "200 OK") != NULL) {
                puts("SUCCESS - The user entered the library successfully.");
            } else {
                puts("ERROR - Could not enter library.");
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            // se extrage token-ul JWT din raspunsul serverului
            char *start_token = strstr(response, "\"token\":\"");
            if (start_token != NULL) {
                // se sare peste "token":"
                start_token += 9;
                char *end_token = strstr(start_token, "\"");
                if (end_token != NULL) {
                    memset(jwt, 0, BUFLEN);
                    memcpy(jwt, start_token, end_token - start_token);
                }
            }

            close_connection(sockfd);
            close(sockfd);
        } else if (strcmp(command, "get_books") == 0) {
            // se verifica daca user-ul este logat
            if (cookies_count == 0) {
                puts("ERROR - User not logged in.");
                continue;
            }

            // se verifica daca user-ul a intrat in biblioteca
            if (strlen(jwt) == 0) {
                puts("ERROR - User not in library.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // compute GET request
            message = compute_get_request(HOST, "/api/v1/tema/library/books", NULL, cookies, cookies_count, jwt);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori
            if (strstr(response, "200 OK") == NULL) {
                puts("ERROR - Could not get books.");
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            // se extrag cartile din raspunsul serverului
            char *books = strstr(response, "[");
            JSON_Value *main_obj = json_parse_string(books);
            JSON_Array *books_arr = json_value_get_array(main_obj);

            if (json_array_get_count(books_arr) == 0) {
                puts("[]");
                json_value_free(main_obj);
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            // se afiseaza cartile in format JSON
            printf("[\n");
            for (int i = 0; i < json_array_get_count(books_arr); i++) {
                JSON_Object *book_obj = json_array_get_object(books_arr, i);
                int id = (int)json_object_get_number(book_obj, "id");
                const char *title = json_object_get_string(book_obj, "title");
                printf("  {\n      \"id\": %d,\n      \"title\": \"%s\"\n  }", id, title);

                if (i != json_array_get_count(books_arr) - 1) {
                    printf(",\n");
                }
            }
            printf("\n]\n");

            json_value_free(main_obj);
            close_connection(sockfd);
            close(sockfd);
        } else if (strcmp(command, "get_book") == 0) {
            // se verifica daca user-ul este logat
            if (cookies_count == 0) {
                puts("ERROR - User not logged in.");
                continue;
            }

            // se verifica daca user-ul a intrat in biblioteca
            if (strlen(jwt) == 0) {
                puts("ERROR - User not in library.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // se citeste id-ul cartii
            char id[10];
            printf("id=");
            fgets(id, 10, stdin);
            id[strlen(id) - 1] = '\0';

            // se verifica daca id-ul este valid
            int flag = 0;
            for (int i = 0; i < strlen(id); i++) {
                if (id[i] < '0' || id[i] > '9') {
                    puts("ERROR - Invalid id.");
                    flag = 1;
                    break;
                }
            }

            // se inchide conexiunea daca id-ul nu este valid
            if (flag == 1) {
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            // se obtine url-ul
            char url[100] = "/api/v1/tema/library/books/";
            strcat(url, id);

            // compute GET request
            message = compute_get_request(HOST, url, id, cookies, cookies_count, jwt);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori
            if (strstr(response, "404 Not Found") != NULL) {
                printf("ERROR - The book with id=%d was not found.", atoi(id));
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            // se extrage cartea din raspunsul serverului
            char *book = strstr(response, "{");
            JSON_Value *main_obj = json_parse_string(book);
            JSON_Object *book_obj = json_value_get_object(main_obj);

            // se afiseaza cartea in format JSON
            int id_book = (int)json_object_get_number(book_obj, "id");
            const char *title = json_object_get_string(book_obj, "title");
            const char *author = json_object_get_string(book_obj, "author");
            const char *publisher = json_object_get_string(book_obj, "publisher");
            const char *genre = json_object_get_string(book_obj, "genre");
            int page_count = (int)json_object_get_number(book_obj, "page_count");

            printf("{\n    \"id\": %d,\n    \"title\": \"%s\"\n", id_book, title);
            printf("    \"author\": \"%s\",\n    \"publisher\": \"%s\"\n", author, publisher);
            printf("    \"genre\": \"%s\",\n    \"page_count\": %d\n}\n", genre, page_count);

            json_value_free(main_obj);
            close_connection(sockfd);
            close(sockfd);
        } else if (strcmp(command, "add_book") == 0) {
            // se verifica daca user-ul este logat
            if (cookies_count == 0) {
                puts("ERROR - User not logged in.");
                continue;
            }

            // se verifica daca user-ul a intrat in biblioteca
            if (strlen(jwt) == 0) {
                puts("ERROR - User not in library.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // se citesc datele cartii
            char title[100];
            printf("title=");
            fgets(title, 100, stdin);
            title[strlen(title) - 1] = '\0';

            char author[50];
            printf("author=");
            fgets(author, 50, stdin);
            author[strlen(author) - 1] = '\0';

            char genre[50];
            printf("genre=");
            fgets(genre, 50, stdin);
            genre[strlen(genre) - 1] = '\0';

            char publisher[50];
            printf("publisher=");
            fgets(publisher, 50, stdin);
            publisher[strlen(publisher) - 1] = '\0';

            char page_count[10];
            printf("page_count=");
            fgets(page_count, 10, stdin);
            page_count[strlen(page_count) - 1] = '\0';

            // se verifica daca numarul de pagini este valid
            int flag = 0;
            for (int i = 0; i < strlen(page_count); i++) {
                if (page_count[i] < '0' || page_count[i] > '9') {
                    puts("ERROR - Invalid page count.");
                    flag = 1;
                    break;
                }
            }

            // daca numarul de pagini nu este valid
            // nu se permite adaugarea cartii
            // se inchide conexiunea
            if (flag == 1) {
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            int page_count_int = atoi(page_count);

            // se construieste obiectul JSON
            JSON_Value *main_obj = json_value_init_object();
            JSON_Object *book_obj = json_value_get_object(main_obj);

            json_object_set_string(book_obj, "title", title);
            json_object_set_string(book_obj, "author", author);
            json_object_set_string(book_obj, "genre", genre);
            json_object_set_string(book_obj, "publisher", publisher);
            json_object_set_number(book_obj, "page_count", atoi(page_count));

            char *ser_data = json_serialize_to_string(main_obj);

            // compute POST request
            message = compute_post_request(HOST, "/api/v1/tema/library/books", "application/json", &ser_data, strlen(ser_data), cookies, cookies_count, jwt);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori
            if (strstr(response, "200 OK") != NULL) {
                puts("SUCCESS - The book has been added successfully.");
            } else {
                puts("ERROR - Could not add book.");
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            free(ser_data);
            json_value_free(main_obj);
            close_connection(sockfd);
            close(sockfd);
        } else if (strcmp(command, "delete_book") == 0) {
            // se verifica daca user-ul este logat
            if (cookies_count == 0) {
                puts("ERROR - User not logged in.");
                continue;
            }

            // se verifica daca user-ul a intrat in biblioteca
            if (strlen(jwt) == 0) {
                puts("ERROR - User not in library.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // se citeste id-ul cartii
            char id[10];
            printf("id=");
            fgets(id, 10, stdin);
            id[strlen(id) - 1] = '\0';

            int flag = 0;
            // se verifica daca id-ul este valid
            for (int i = 0; i < strlen(id); i++) {
                if (id[i] < '0' || id[i] > '9') {
                    puts("ERROR - Invalid id.");
                    flag = 1;
                    break;
                }
            }

            // se inchide conexiunea daca id-ul nu este valid
            if (flag == 1) {
                close_connection(sockfd);
                close(sockfd);
                continue;
            }

            // se obtine url-ul
            char url[100] = "/api/v1/tema/library/books/";
            strcat(url, id);

            // compute DELETE request
            message = compute_del_request(HOST, url, id, cookies, cookies_count, jwt);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori
            if (strstr(response, "404 Not Found") != NULL) {
                puts("ERROR - No book was deleted.");
                close_connection(sockfd);
                close(sockfd);
                continue;
            } else {
                puts("SUCCESS - The book was successfully deleted.");
            }

            close_connection(sockfd);
            close(sockfd);
        } else if (strcmp(command, "logout") == 0) {
            // se verifica daca user-ul este logat
            if (cookies_count == 0) {
                puts("ERROR - User not logged in.");
                continue;
            }

            // se deschide conexiunea cu serverul
            int sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

            // compute GET request
            message = compute_get_request(HOST, "/api/v1/tema/auth/logout", NULL, cookies, cookies_count, jwt);
            // se trimite mesajul la server
            send_to_server(sockfd, message);
            // se primeste raspunsul de la server
            response = receive_from_server(sockfd);

            // se verifica de erori
            if (strstr(response, "200 OK") != NULL) {
                puts("SUCCESS - User logged out successfully.");
            } else {
                puts("ERROR - Could not logout.");
            }

            // se sterg cookie-urile sesiunii curente
            for (int i = 0; i < cookies_count; i++) {
                free(cookies[i]);
            }
            cookies_count = 0;
            close_connection(sockfd);
            close(sockfd);
        } else {
            puts("Invalid command.");
        }
    }

    free(cookies);
    free(jwt);
    free(message);
    free(response);
    return 0;
}
