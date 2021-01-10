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
#include "parson.c"

#define BUFFLEN 500

//alcatuiesc header-ul corespunzator GET-ului
char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookie, char* command, char* token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    //scrie numele metodei, URL si toipul protocolului
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    //adauga host-ul
    sprintf(line, "HOST: %s", host);
    compute_message(message, line);

    //completeaza cookie-ul
    if(strncmp(command, "enter_library", 13) == 0 || strncmp(command, "logout", 6) == 0){
        sprintf(line, "Cookie: %s", cookie);
        compute_message(message, line);
    }

    //completeaza campul de autorizare
    if(strncmp(command, "get_books", 9) == 0 || strncmp(command, "get_book", 8) == 0){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    //adauga linie goala la finalul header-ului
    compute_message(message, "");

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char *query_params, char* token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    //scrie numele metodei, URL si toipul protocolului
    if (query_params != NULL) {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url);
    }

    compute_message(message, line);

    //adauga host-ul
    sprintf(line, "HOST: %s", host);
    compute_message(message, line);
   
    //completeaza campul de autorizare
    sprintf(line, "Authorization: Bearer %s", token);
    compute_message(message, line);

    //adauga linie goala la finalul header-ului
    compute_message(message, "");

    free(line);
    return message;
}

//alcatuiesc header-ul corespunzator POST-ului
char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
                            int body_data_fields_count, char* command, char* token)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));
    int len = 0;

    //scrie numele metodei, URL si toipul protocolului
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    //adauga host-ul
    sprintf(line, "HOST: %s", host);
    compute_message(message, line);
   
    //adauga token-ul in Authorization
    if(strncmp(command, "add_book", 8) == 0) {
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }

    //scrie Content-Type-ul
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;

    //formez stringul pe care il trimit catre server
    if(strncmp(command, "register", 8) == 0 || strncmp(command, "login", 5) == 0){
        json_object_set_string(root_object, "username", body_data[0]);
        json_object_set_string(root_object, "password", body_data[1]); 
    }

    if(strncmp(command, "add_book", 8) == 0){
        json_object_set_string(root_object, "title", body_data[0]);
        json_object_set_string(root_object, "author", body_data[1]); 
        json_object_set_string(root_object, "genre", body_data[2]);
        json_object_set_string(root_object, "publisher", body_data[3]); 
        json_object_set_number(root_object, "page_count", atoi(body_data[4])); 
    }
    serialized_string = json_serialize_to_string_pretty(root_value); 

    strcpy(body_data_buffer, serialized_string);
    json_free_serialized_string(serialized_string); 
    json_value_free(root_value);

    //calzuleaza dimensiunea stringului
    len = strlen(body_data_buffer);

    //completeaza Content-Length
    sprintf(line, "Content-Length: %d", len);
    compute_message(message, line);

    //adauga o linie goala la finalul stringului
    compute_message(message, "");

    //adauga payload-ul
    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    //eliberare memorie
    free(body_data_buffer);
    free(line);
    return message;
}

//extrag token-ul din raspunsul primit de la server
char* get_token(char *response){
    char* jwt_token_aux = malloc(BUFFLEN*sizeof(char));
    char* jwt_token = malloc(BUFFLEN*sizeof(char));

    //iau stringul in forma json
    const char s[2] = "\n";
    char *token;
    token = strtok(response, s);

    while( token != NULL ) {
        strcpy(jwt_token_aux, token);
        token = strtok(NULL, s);
    }

    //extrag token-ul cerut
    JSON_Value *root_value = json_parse_string(jwt_token_aux);
    JSON_Object *obj = json_value_get_object (root_value);
    char* token_aux = malloc(BUFFLEN*sizeof(char));
    strcpy(token_aux, "token");
    const char  *val  = json_object_get_string (obj, token_aux);
    strcpy(jwt_token, val);

    //eliberez memoria
    json_value_free(root_value);
    free(jwt_token_aux);
    free(token_aux);

    return jwt_token;
}
