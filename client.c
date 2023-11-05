#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>

#include "helpers.h"
#include "requests.h"
#include "parson.h"

int isValidUsername(const char *username)
{
    int i = 0;
    while (username[i] != '\0')
    {
        if (username[i] == ' ')
        {
            return 0;
        }
        i++;
    }
    return 1;
}

void register_func(int sockfd, char *host, char **cookie)
{
    if (*cookie != NULL)
    {
        printf("Esti deja logat!\n\n");
        return;
    }

    char *username = (char *)calloc(30, sizeof(char));
    DIE(username == NULL, "Eroare calloc username.\n");
    char *password = (char *)calloc(30, sizeof(char));
    DIE(password == NULL, "Eroare calloc password.\n");

    printf("username=");
    fgets(username, LINELEN, stdin);
    username[strlen(username) - 1] = '\0';
    printf("password=");
    fgets(password, LINELEN, stdin);
    password[strlen(password) - 1] = '\0';

    if (isValidUsername(username) == 0 || isValidUsername(username) == 0)
    {
        printf("Username-ul si parola nu trebuie sa contina spatii.\n\n");
        return;
    }

    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    json_object_set_string(object, "password", password);

    char *body = json_serialize_to_string(value);

    char *message = compute_post_request(host, "/api/v1/tema/auth/register", "application/json", &body, 1, NULL, 0, NULL);
    // printf("REGISTER: compute_post_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("REGISTER: receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "is taken") != NULL)
    {
        printf("Exista deja cineva.\n\n");
    }
    else
    {
        printf("Te-ai inregistrat cu succes <333!\n\n");
    }

    free(username);
    free(password);
    json_value_free(value);
    free(body);
    free(message);
    free(answer);

    return;
}

void login_func(int sockfd, char *host, char **cookie)
{
    if (*cookie != NULL)
    {
        printf("Esti deja logat!\n\n");
        return;
    }

    char *username = (char *)calloc(30, sizeof(char));
    DIE(username == NULL, "Eroare calloc username.\n\n");
    char *password = (char *)calloc(30, sizeof(char));
    DIE(password == NULL, "Eroare calloc password.\n\n");

    printf("username=");
    fgets(username, LINELEN, stdin);
    username[strlen(username) - 1] = '\0';
    printf("password=");
    fgets(password, LINELEN, stdin);
    password[strlen(password) - 1] = '\0';

    if (isValidUsername(username) == 0 || isValidUsername(username) == 0)
    {
        printf("Username-ul si parola nu trebuie sa contina spatii.\n\n");
        return;
    }

    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "username", username);
    json_object_set_string(object, "password", password);

    char *body = json_serialize_to_string(value);

    char *message = compute_post_request(host, "/api/v1/tema/auth/login", "application/json", &body, 1, NULL, 0, NULL);
    // printf("LOGIN: compute_post_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("LOGIN: receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "OK") == NULL)
    {
        *cookie = NULL;
        printf("login esuat :(\n\n");
        return;
    }

    char *start_cookie = strstr(answer, "connect.sid");
    char *my_cookie = strtok(start_cookie, ";");
    *cookie = calloc(strlen(my_cookie), sizeof(char));
    memcpy(*cookie, my_cookie, strlen(my_cookie));

    printf("Te-ai logat cu succes <333 !!!\n\n");
    free(username);
    free(password);
    json_value_free(value);
    free(body);
    free(message);
    free(answer);
    return;
}

void enter_lib(int sockfd, char *host, char **cookie, char **token)
{

    if (*cookie == NULL)
    {
        printf("Nu esti logat!\n\n");
        return;
    }

    if (*token != NULL)
    {
        printf("Ai accesat deja biblioteca\n\n");
        return;
    }

    char *message = compute_get_request(host, "/api/v1/tema/library/access", NULL, &(*cookie), NULL, 1);
    // printf("ENTER LIB : compute_get_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("ENTER LIB: receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "OK") == NULL)
    {
        printf("accesare biblioteca esuata :(\n\n");
        free(message);
        free(answer);
        return;
    }

    const char *tokenKey = "\"token\":\"";
    char *start_token = strstr(answer, tokenKey);
    start_token += strlen(tokenKey);
    char *my_token = strtok(start_token, "\"");
    *token = calloc(strlen(my_token), sizeof(char));
    memcpy(*token, my_token, strlen(my_token));

    printf("Ai accesat biblioteca cu succes <333 !\n\n");

    free(message);
    free(answer);

    return;
}

void get_books(int sockfd, char *host, char **cookie, char **token)
{
    if (*cookie == NULL)
    {
        printf("Nu esti logat!\n\n");
        return;
    }
    if (*token == NULL)
    {
        printf("Nu ai accesat biblioteca!\n\n");
        return;
    }
    char *message = compute_get_request(host, "/api/v1/tema/library/books", NULL, NULL, &(*token), 1);
    // printf("GET BOOKS : compute_get_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("GET BOOKS : receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "OK") == NULL)
    {
        printf("eroare get books\n\n");
        free(message);
        free(answer);
        return;
    }

    const char *Key = "[";
    char *start = strstr(answer, Key);
    start += strlen(Key);
    char *my_books = strtok(start, "]");
    if (my_books == NULL)
        printf("gol golut...\n");
    else
        printf("Ia citeste de aici : %s\n\n", my_books);
    return;
}

void get_book(int sockfd, char *host, char **cookie, char **token)
{
    if (*cookie == NULL)
    {
        printf("Nu esti logat!\n\n");
        return;
    }

    if (*token == NULL)
    {
        printf("Nu ai accesat biblioteca!\n\n");
        return;
    }
    char id[10];
    printf("id= ");
    scanf("%s", id);
    getchar();

    int digits;
    char *str;
    digits = strtol(id, &str, 10);
    if (*str != '\0')
    {
        printf("id incorect. Accept doar cifre!\n\n");
        return;
    }

    char *dest = calloc(50, sizeof(char));
    strcat(dest, "/api/v1/tema/library/books/");
    strcat(dest, id);
    char *message = compute_get_request(host, dest, NULL, NULL, &(*token), 1);
    // printf("GET BOOK : compute_get_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("GET BOOK : receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "OK") == NULL)
    {
        printf("eroare get_book\n\n");
        free(message);
        free(answer);
        return;
    }

    char *book = basic_extract_json_response(answer);
    if (book == NULL)
        printf("nu am gasit cartea :(\n\n");
    else
        printf("ia si citeste : %s\n\n", book);
}

void add_book(int sockfd, char *host, char **cookie, char **token)
{
    if (*cookie == NULL)
    {
        printf("Nu esti logat!\n\n");
        return;
    }

    if (*token == NULL)
    {
        printf("Nu ai accesat biblioteca\n\n");
        return;
    }

    char *title = (char *)calloc(LINELEN, sizeof(char));
    DIE(title == NULL, "Eroare calloc title.\n");
    char *author = (char *)calloc(LINELEN, sizeof(char));
    DIE(author == NULL, "Eroare calloc author.\n");
    char *genre = (char *)calloc(LINELEN, sizeof(char));
    DIE(genre == NULL, "Eroare calloc genre.\n");
    char *publisher = (char *)calloc(LINELEN, sizeof(char));
    DIE(publisher == NULL, "Eroare calloc publisher.\n");
    char *page_count = (char *)calloc(LINELEN, sizeof(char));
    DIE(page_count == NULL, "Eroare calloc page_count.\n");

    printf("title=");
    fgets(title, LINELEN, stdin);
    title[strlen(title) - 1] = '\0';

    printf("author=");
    fgets(author, LINELEN, stdin);
    author[strlen(author) - 1] = '\0';

    printf("genre=");
    fgets(genre, LINELEN, stdin);
    genre[strlen(genre) - 1] = '\0';

    printf("publisher=");
    fgets(publisher, LINELEN, stdin);
    publisher[strlen(publisher) - 1] = '\0';

    printf("page_count=");
    fgets(page_count, LINELEN, stdin);
    page_count[strlen(page_count) - 1] = '\0';

    int digits;
    char *str;
    digits = strtol(page_count, &str, 10);

    if (*str != '\0')
    {
        printf("page_count incorect. Accept doar cifre\n\n");
        return;
    }

    JSON_Value *value = json_value_init_object();
    JSON_Object *object = json_value_get_object(value);
    json_object_set_string(object, "title", title);
    json_object_set_string(object, "author", author);
    json_object_set_string(object, "genre", genre);
    json_object_set_number(object, "page_count", digits);
    json_object_set_string(object, "publisher", publisher);

    char *body = json_serialize_to_string(value);

    char *message = compute_post_request(host, "/api/v1/tema/library/books", "application/json", &body, 1, NULL, 0, &(*token));
    // printf("LOGIN: compute_post_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("LOGIN: receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "OK") == NULL)
    {
        printf("eroare add_book !\n\n");
        free(author);
        free(genre);
        free(publisher);
        free(title);
        free(body);
        free(value);
        free(answer);
        free(message);
        return;
    }

    printf("cartea ta a fost adaugata cu succes <333 !\n\n");
    free(author);
    free(genre);
    free(publisher);
    free(title);
    free(body);
    free(value);
    free(answer);
    free(message);
    return;
}

void delete_book(int sockfd, char *host, char **cookie, char **token)
{
    if (*cookie == NULL)
    {
        printf("Nu esti logat!\n\n");
        return;
    }

    if (*token == NULL)
    {
        printf("Nu ai accesat biblioteca\n\n");
        return;
    }
    char id[10];
    printf("id= ");
    scanf("%s", id);
    getchar();

    int digits;
    char *str;
    digits = strtol(id, &str, 10);
    if (*str != '\0')
    {
        printf("id incorect\n\n");
        return;
    }

    char *dest = calloc(50, sizeof(char));
    strcat(dest, "/api/v1/tema/library/books/");
    strcat(dest, id);
    char *message = compute_delete_request(host, dest, NULL, NULL, &(*token), 1);
    // printf("DELETE BOOK : compute_delete_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("DELETE BOOK : receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "OK") == NULL)
    {
        printf("eroare get books\n\n");
        free(message);
        free(answer);
        return;
    }

    printf("Gata, nu o sa mai auzi de cartea asta :) !\n\n");
    free(message);
    free(answer);
    return;
}

void log_out(int sockfd, char *host, char **cookie, char **token)
{
    if (*cookie == NULL)
    {
        printf("Nu esti logat!\n\n");
        return;
    }
    char *message = compute_get_request(host, "/api/v1/tema/auth/logout", NULL, &(*cookie), NULL, 1);
    // printf("LOG OUT : compute_get_request output : \n%s\n", message);
    send_to_server(sockfd, message);
    char *answer = receive_from_server(sockfd);
    // printf("LOG OUT : receive_from_server output : \n%s\n", answer);

    if (strstr(answer, "OK") == NULL)
    {
        printf("eroare get_books\n\n");
        free(message);
        free(answer);
        return;
    }

    printf("nu mai e nimeni pe aici :(\n\n");
    *cookie = NULL;
    *token = NULL;
    free(message);
    free(answer);
    return;
}

int main(int argc, char *argv[])
{
    char line[100];

    char host[16] = "34.254.242.81";
    int port = 8080;

    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    char *cookie = NULL; // pentru verificare: client logat?
    char *token = NULL;
    while (1)
    {

        // printf("Ce vrei sa facem azi?\n");
        fgets(line, 100, stdin);

        if (strncmp(line, "exit", 4) == 0)
        {
            printf("imediat ies...\n");
            break;
        }
        else
        {

            sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
            DIE(sockfd < 0, "eroare open_connection\n");

            if (strncmp(line, "register", 8) == 0)
            {
                printf("te inregistrez acum...\n");
                register_func(sockfd, host, &cookie);
            }

            else if (strncmp(line, "login", 5) == 0)
            {
                printf("verificam daca esti tu...\n");
                login_func(sockfd, host, &cookie);
                // intoarce cookie de sesiune
            }
            else if (strncmp(line, "enter_library", 13) == 0)
            {
                printf("accesez biblioteca...\n");
                enter_lib(sockfd, host, &cookie, &token);
            }
            else if (strncmp(line, "get_books", 9) == 0)
            {
                printf("am plecat dupa carti...\n");
                get_books(sockfd, host, &cookie, &token);
                // close(sockfd)
            }
            else if (strncmp(line, "get_book", 8) == 0)
            {
                printf("ma duc sa caut cartea ta...\n");
                get_book(sockfd, host, &cookie, &token);
            }
            else if (strncmp(line, "add_book", 8) == 0)
            {
                printf("acum adaug cartea...\n");
                add_book(sockfd, host, &cookie, &token);
            }
            else if (strncmp(line, "delete_book", 11) == 0)
            {
                printf("Ard cartea si fac gratar din ea...\n");
                delete_book(sockfd, host, &cookie, &token);
            }
            else if (strncmp(line, "logout", 6) == 0)
            {
                printf("uofff...pregatesc plecarea...\n");
                log_out(sockfd, host, &cookie, &token);
            }

            close(sockfd);
        }
    }
    // return 0;
}
