#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <stdbool.h> 
#include "helpers.h"
#include "requests.h"

#define HOST "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define CONTENT_TYPE "application/json"
#define BUFFLEN 500

void register_command(int sockfd, char* command) {
	char *message;
    char *response;
	int body_data_fields_count = 2;
    char **body_data = (char**)malloc(body_data_fields_count*sizeof(char*));
    for(int i = 0; i < body_data_fields_count; i++) {
        body_data[i] = (char*)malloc(sizeof(char));
    }

    //setez campurile din body_data cu username-ul si parola introduse
    printf("username=");
    scanf("%s", body_data[0]);
   
    printf("password=");
    scanf("%s", body_data[1]);

    //creez hederul pe care il trimit la server
    message = compute_post_request(HOST, "/api/v1/tema/auth/register", CONTENT_TYPE,
    					 body_data, body_data_fields_count, command, NULL);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response); //afizes mesajul primit de la server

    //eliberare memorie
    for(int i = 0; i < body_data_fields_count; i++)
        free(body_data[i]);
    free(body_data);
    free(message);
    free(response);
}

//extrag cookie-ul din raspunsul server-ului
char* get_cookie(char* response) {
	const char s[4] = "\r\n ";
	int incorrect_login = 0;
   	char *token;
   	char* cookie = malloc(250*sizeof(char));
   	token = strtok(response, s);

    while( token != NULL ) {
	   	if(strcmp(token, "Set-Cookie:") == 0) {
		    token = strtok(NULL, s);
		    strcpy(cookie, token);
		    strcat(cookie, " ");
		    token = strtok(NULL, s);
		    strcat(cookie, token);
		    strcat(cookie, " ");
		    token = strtok(NULL, s);
		    strcat(cookie, token);
		    strcat(cookie, "\n");
		    incorrect_login = 1;
		    break;
	   	}
	   	token = strtok(NULL, s);
    }

    //verific daca conectarea s-a realizat cu succes; in cazul in care
    //nu primesc niciun cookie, intorc un mesaj
    if(incorrect_login == 0)
    	strcpy(cookie, "incorrect_login");

    return cookie;
}

char* login_command(int sockfd, char* command){
	char *message;
    char *response;
    char *cookie;
	int body_data_fields_count = 2;
    char **body_data = (char**)malloc(body_data_fields_count*sizeof(char*));

    for(int i = 0; i < body_data_fields_count; i++) {
        body_data[i] = (char*)malloc(sizeof(char));
    }

    //setez campurile din body_data cu username-ul si parola introduse
    printf("username=");
    scanf("%s", body_data[0]);
   
    printf("password=");
    scanf("%s", body_data[1]);
  
  	//creez headerul pe care il trimit serverului
    message = compute_post_request(HOST, "/api/v1/tema/auth/login", CONTENT_TYPE, 
    							body_data, body_data_fields_count, command, NULL);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response); //afizes mesajul primit de la server

    cookie = get_cookie(response);

    //eliberare memorie
    for(int i = 0; i < body_data_fields_count; i++)
        free(body_data[i]);
    free(body_data);
    free(message);
    free(response);

    return cookie;
}

//verific daca un string este numar
bool check_number(char* string) {
	for(int i = 0; i < strlen(string); i++)
		 if (string[i] < 48 || string[i] > 57)
         	return false;
    return true;
}

void add_book_command(int sockfd, char* token, char* command) {
	char *message;
    char *response;
    char *buffer = malloc(BUFFLEN*sizeof(char));
    char *buffer_aux = malloc(BUFFLEN*sizeof(char));
	int body_data_fields_count = 5;
    char **body_data = (char**)malloc(body_data_fields_count*sizeof(char*));

    for(int i = 0; i < body_data_fields_count; i++) {
        body_data[i] = (char*)malloc(sizeof(char));
    }

    //pun informatiile referitoare la o anumita carte in body_data
    printf("title=");
    memset(buffer, 0, BUFFLEN);
    fgets(buffer, BUFFLEN - 1, stdin);
    strcpy(body_data[0],strtok(buffer, "\n"));

    printf("author=");
    memset(buffer, 0, BUFFLEN);
    fgets(buffer, BUFFLEN - 1, stdin);
    strcpy(body_data[1], strtok(buffer, "\n"));

    printf("genre=");
    memset(buffer, 0, BUFFLEN);
    fgets(buffer, BUFFLEN - 1, stdin);
    strcpy(body_data[2],strtok(buffer, "\n"));
    
    printf("publisher=");
    memset(buffer, 0, BUFFLEN);
    fgets(buffer, BUFFLEN - 1, stdin);
    strcpy(body_data[3], strtok(buffer, "\n"));

    printf("page_count=");
    memset(buffer, 0, BUFFLEN);
    fgets(buffer, BUFFLEN - 1, stdin);
    strcpy(buffer_aux, strtok(buffer, "\n"));
    
    //doar in cazul in care informatiile respecta formatul, trimit mesaj server-ului
    if(check_number(buffer_aux) == true) {
    	strcpy(body_data[4], buffer_aux);
	    //creez hederul pe care il trimit server-ului
	    message = compute_post_request(HOST, "/api/v1/tema/library/books", CONTENT_TYPE,
	    							body_data, body_data_fields_count, command, token);
	    puts(message);
	    send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);
	    puts(response); // afisez raspunsul primit de la server
	    free(message);
	    free(response);
    } else {
    	printf("Invalid page_count!\n");
    }

    //eliberez memoria
    for(int i = 0; i < body_data_fields_count; i++)
        free(body_data[i]);
    free(body_data);
    free(buffer);
    free(buffer_aux);
}

char* acces_command(char* cookie, int sockfd, char* command){
	char *message;
    char *response;
    char* jwt_token;

    //creez hederul pe care il trimit server-ului
	message = compute_get_request(HOST, "/api/v1/tema/library/access", NULL, 
													cookie, command, NULL);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);

    jwt_token = get_token(response);
 	
 	//eliberez memoria
 	free(message);
    free(response);

	return jwt_token; //intorc token-ul
}

void get_books_command(int sockfd, char* token, char* command){
	char *message;
    char *response;
  	
  	//creez hederul pe care il trimit server-ului
	message = compute_get_request(HOST, "/api/v1/tema/library/books", 
											NULL, NULL, command, token);
    puts(message);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);

    //eliberez memoria
    free(message);
    free(response);
}

void get_book_command(int sockfd, char* token, char* command){
	char *message;
    char *response;
    char *buffer = malloc(BUFFLEN*sizeof(char));
    char *buffer_aux = malloc(BUFFLEN*sizeof(char));
    char* url = malloc(BUFFLEN*sizeof(char));
    strcpy(url, "/api/v1/tema/library/books/");

    printf("id=");
   	memset(buffer, 0, BUFFLEN);
    fgets(buffer, BUFFLEN - 1, stdin);
    memset(buffer_aux, 0, BUFFLEN);
    memcpy(buffer_aux, buffer, strlen(buffer) - 1);

    //verific daca id-ul introdus are formatul cerut
    if(buffer[0] != '\n' && check_number(buffer_aux) == true) {
    	strcat(url, buffer_aux);
		message = compute_get_request(HOST, url, NULL, NULL, command, token);
	    puts(message);
	    send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);
	    puts(response);

	    free(message);
	    free(response);
    } else {
    	printf("Invalid id!\n");
    }

    //eliberez memoria
    free(buffer);
    free(buffer_aux);
    free(url); 
}

void delete_book_command(int sockfd, char* token, char* command){
	char *message;
    char *response;
    char *buffer = malloc(BUFFLEN*sizeof(char));
    char *buffer_aux = malloc(BUFFLEN*sizeof(char));
    char* url = malloc(BUFFLEN*sizeof(char));
    strcpy(url, "/api/v1/tema/library/books/");

    printf("id=");
    memset(buffer, 0, BUFFLEN);
    fgets(buffer, BUFFLEN - 1, stdin);
    memset(buffer_aux, 0, BUFFLEN);
    memcpy(buffer_aux, buffer, strlen(buffer) - 1);
    
    //verific daca id-ul introdus are formatul cerut
    if(buffer[0] != '\n' && check_number(buffer_aux) == true) {
    	strcat(url, buffer_aux);
		message = compute_delete_request(HOST, url, NULL, token);
	    puts(message);
	    send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);
	    puts(response);

	    free(message);
	    free(response);
    } else {
    	printf("Invalid id!\n");
    }

    //eliberez memoria
    free(buffer);
    free(buffer_aux);
    free(url); 
}

void logout_command(int sockfd, char* cookie, char* command){
	char *message;
    char *response;
   
   	//creez head-ul pe care ulterior il trimit server-ului
	message = compute_get_request(HOST, "/api/v1/tema/auth/logout", 
									NULL, cookie, command, NULL);
    puts(message);
    send_to_server(sockfd, message);//trimit mesaj server-ului
    response = receive_from_server(sockfd);
    puts(response);
    
    //eliberez memoria
    free(message);
    free(response);
}
