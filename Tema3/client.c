#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <stdbool.h> 
#include "commands.h"


int main(int argc, char *argv[])
{
 
    int sockfd;
    int single_login = 0;
    int access = 0;
    int bufflen = 25;
    char* command = malloc(bufflen*sizeof(char));
    char* jwt_token;
    char* cookie;
    
    while(1){
    	fgets(command, bufflen - 1, stdin);
    	sockfd = open_connection("3.8.116.10", 8080, AF_INET, SOCK_STREAM, 0);

    	if(strncmp(command, "register", 8) == 0){
    		register_command(sockfd, command);
    	}
    	if(strncmp(command, "login", 5) == 0){
    		if(single_login == 0) { //permite conectarea unui singur utilizator
    			cookie = login_command(sockfd, command);
	    		if(strncmp(cookie, "incorrect_login", 15))
	    			single_login = 1;
    		}
    		else
    			printf("You are already logged in!\n");
    	}
    	if(strncmp(command, "enter_library", 13) == 0){
    		if(single_login == 1) {
	    		jwt_token = acces_command(cookie, sockfd, command);
	    		access = 1;
    		}
    		else
    			printf("You are not logged in!\n");
    	}
    	if(strncmp(command, "add_book", 8) == 0){
    		if(access == 1)
       			add_book_command(sockfd, jwt_token, command);
       		else
       			printf("You do not have access!\n");
    	}
    	if(strncmp(command, "get_books", 9) == 0){
    		if(access == 1)
    			get_books_command(sockfd, jwt_token, command);	
       		else
       			printf("You do not have access!\n");
    	} else if(strncmp(command, "get_book", 8) == 0){
    		if(access == 1)
    			get_book_command(sockfd, jwt_token, command);	
       		else
       			printf("You do not have access!\n");
    	}
    	if(strncmp(command, "delete_book", 6) == 0){
    		if(access == 1)
    			delete_book_command(sockfd, jwt_token, command);	
       		else
       			printf("You do not have access!\n");
    	}
    	if(strncmp(command, "logout", 6) == 0){
    		if(single_login == 1) {
    			logout_command(sockfd, cookie, command);
    			single_login = 0;
    			access = 0;	
    			free(cookie);
			    free(jwt_token);
    		}
    		else
    			printf("You are not logged in!\n");
    	}
    	if(strncmp(command, "exit", 4) == 0)
    		break;

    }

    free(command);
    close(sockfd);
    return 0;
}
