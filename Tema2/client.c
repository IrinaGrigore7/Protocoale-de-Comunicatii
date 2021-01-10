#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <math.h>
#include <netinet/tcp.h>
#include "helpers.h"
#include "utils.h"

void usage(char *file) {
    fprintf(stderr, "Usage: %s server_address server_port\n", file);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, ret, flag = 1;
    struct sockaddr_in serv_addr;
    char buffer[BUFLEN];

    fd_set read_fds;
    fd_set tmp_fds;

    int fdmax;

    FD_ZERO(&tmp_fds);
    FD_ZERO(&read_fds);

    if (argc < 4) {
        usage(argv[0]);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    FD_SET(sockfd, &read_fds);
    fdmax = sockfd;
    FD_SET(0, &read_fds);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    ret = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(ret == 0, "inet_aton");

    ret = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "connect");

    //trimit catre server id-ul clientului
    ret = send(sockfd, argv[1], strlen(argv[1])+ 1, 0);
    DIE(ret < 0, "No client ID");

    //dezactivez algoritmul lui Nagle
    int result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
    DIE(result < 0, "setsockopt");

    while (1) {
        tmp_fds = read_fds;

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        if (FD_ISSET(0, &tmp_fds)) {  // citeste de la tastatura
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);
            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            }else if(strncmp(buffer, "subscribe", 9) == 0) {
                //completez campurile din structura tcp_message
                const char s[2] = " ";
                char *token;
                token = strtok(buffer, s);
                int index = 0;//0-subscribe; 1-topic; 2-SF
                struct tcp_message tcp_msg;
                while( token != NULL ) {
                    switch(index) {
                        case 0:
                            tcp_msg.type = 1;
                            break;
                        case 1:
                            strcpy(tcp_msg.topic, token);
                            break;
                        case 2:
                            tcp_msg.flag = atoi(token);
                            break;
                    }
                    index++;
                    token = strtok(NULL, s);
                    
                }
                ret = send(sockfd, &tcp_msg, sizeof(struct tcp_message), 0);
                DIE(ret < 0, "send");
            } else if(strncmp(buffer, "unsubscribe", 11) == 0) {
                const char s[2] = " ";
                char *token;
                token = strtok(buffer, s);
                int index = 0;//0-subscribe; 1-topic
                struct tcp_message *tcp_msg = malloc(sizeof(struct tcp_message));
                while( token != NULL ) {
                    switch(index) {
                        case 0:
                            tcp_msg->type = 0;
                            break;
                        case 1:
                            strcpy(tcp_msg->topic, token);
                            break;
                    }
                    index++;
                    token = strtok(NULL, s);
                    
                }
                ret = send(sockfd, tcp_msg, sizeof(struct tcp_message), 0);
                DIE(ret < 0, "send");
            } else
                printf("Invalid command\n");

        }
        if (FD_ISSET(sockfd, &tmp_fds)) { //primeste ceva de la server
            struct udp_message udp_msg;
            ret = recv(sockfd, &udp_msg, sizeof(udp_msg), 0);
            DIE(ret < 0, "receive");
            uint32_t int_number;
            double short_number;
            char* number = malloc(1500*sizeof(char));
            char *type = malloc(sizeof(char));
            if(udp_msg.data_type > 3)
                printf("Incorrect type\n");
            // in momentul in care serverul primeste comanda exit, trebuie sa 
            // inchida toti clientii conectati
            if(strncmp(udp_msg.payload, "exit", 4) == 0){
                break;
            }
            //un client incearca sa se conecteze cu un id deja folosit
            if(strncmp(udp_msg.payload, "id", 2) == 0){
                printf("This ID is already used\n");
                break;
            }
            // se compune mesajul pe care clientul il primeste de la clientul udp
            // si pe care trebuie sa-l afiseze
            switch (udp_msg.data_type) {
                case 0:
                    if(udp_msg.payload[0] > 1)
                        printf("Incorrect sign byte\n");
                    int_number = ntohl(*(uint32_t*)(udp_msg.payload + 1));
                    if(udp_msg.payload[0] == 1)
                        int_number = (-1)*int_number;
                    sprintf(number, "%d", int_number);
                    strcpy(type, "INT");
                    break;
                case 1:
                    short_number = ntohs(*(uint16_t*)(udp_msg.payload));
                    short_number /= 100;
                    sprintf(number, "%.2f", short_number);
                    strcpy(type, "SHORT_REAL");
                    break;
                case 2:
                    if(udp_msg.payload[0] > 1)
                        printf("Incorrect sign byte\n");
                    short_number = ntohl(*(uint32_t*)(udp_msg.payload + 1));
                    short_number /= pow(10, udp_msg.payload[5]);
                    if(udp_msg.payload[0] == 1)
                        short_number = (-1)*short_number;
                    sprintf(number, "%lf", short_number);
                    strcpy(type, "FLOAT");
                    break;
                default:
                    strncpy(number, udp_msg.payload, 1500);
                    strcpy(type, "STRING");
            }
            printf("%s:%d - %s - %s - %s\n", udp_msg.ip, udp_msg.udp_port, udp_msg.topic,
                type, number);
        } 
    }

    close(sockfd);
    return 0;
}