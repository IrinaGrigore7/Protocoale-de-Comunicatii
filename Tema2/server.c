#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <netinet/tcp.h>
#include "helpers.h"
#include "utils.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}


int main(int argc, char *argv[])
{
	int tcp_socket, newtcp_socket, port_number;
	int udp_socket;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr, udp_addr;
	int i, ret;
	struct client_struct *clients_connected = malloc(sizeof(struct client_struct));
	struct client_struct *clients_disconnected = malloc(sizeof(struct client_struct));
	socklen_t clilen, udplen; 
	int flag = 1;

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax = 0;			// valoare maxima fd din multimea read_fds

	if (argc < 2) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si 
	//multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	//creez socket-ul pasiv de tcp
	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_socket < 0, "socket");

	//creez socket-ul de udp pe care primesc mesaje
	udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(udp_socket < 0, "UDP socket");

	port_number= atoi(argv[1]);
	DIE(port_number == 0, "atoi");

	//setez campurile din ser_addr
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	memset((char *) &udp_addr, 0, sizeof(udp_addr));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(port_number);
	udp_addr.sin_addr.s_addr = INADDR_ANY;

	//fac bind socket-ului de tcp
	ret = bind(tcp_socket, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "tcp bind");

	//fac bind socket-ului de udp
	ret = bind(udp_socket, (struct sockaddr*) &udp_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "udp bind");

	//se asculta conexiuni de pe sochetul pasiv de tcp
	ret = listen(tcp_socket, MAX_CLIENTS);
	DIE(ret < 0, "listen");

	// se adauga file descriptorul de tcp, precum si cel de udp in multimea read_fds
	FD_SET(tcp_socket, &read_fds);
	FD_SET(udp_socket, &read_fds);
	FD_SET(0, &read_fds);

	if(tcp_socket > fdmax)
		fdmax = tcp_socket;

	//dezactivez algoritmul nui Nagle
	int result = setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, (char*) &flag, sizeof(int));
	DIE(result < 0, "setsockopt");

	if(udp_socket > fdmax)
		fdmax = udp_socket;

	int index = 0; //retin numarul clientilor care s-au conectat
	int index_disc = 0; //retin numarul clientilor deconectati

	while (1) {
		tmp_fds = read_fds; 
		memset(buffer, 0, BUFLEN);
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(0, &tmp_fds))  { //in cazul in care serverul primeste comanda exit
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);
			//trimit un mesaj fiecarui socket conectat pentru a-l inchide
			if (strncmp(buffer, "exit", 4) == 0) {
				for(int j = 1; j <= fdmax; j++) { //parcurg toti socketii
					if(FD_ISSET(j, &read_fds) && (j != tcp_socket) && (j != udp_socket)) {
						struct udp_message udp_msg;
						memset(&udp_msg, 0, sizeof(udp_msg));
						strcpy(udp_msg.payload, "exit");
						ret = send(j, &udp_msg, sizeof(struct udp_message), 0);
						DIE(ret < 0, "Send failed");
					}
				}
				break;
			} 
            else
                printf("Invalid command.\n");
        }

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
                if (i == udp_socket) { //am primit ceva de pe un socket udp
					memset(buffer, 0, BUFLEN);
					udplen = sizeof(struct sockaddr);
					struct udp_message udp_msg;
					memset(&udp_msg, 0, sizeof(udp_msg));
					ret = recvfrom(udp_socket, &udp_msg, sizeof(udp_msg), 0, 
										(struct sockaddr*)&udp_addr, &udplen);
					DIE(ret < 0, "receive");
					strcpy(udp_msg.ip, inet_ntoa(udp_addr.sin_addr));
					udp_msg.udp_port = udp_addr.sin_port;
					//trimit mesajul tuturor clientilor conectati la un anumit topic 
					for(int k = 0; k < index; k++) {
						int contor = clients_connected[k].contor;
						for(int j = 0; j < contor; j++)
							if(strcmp(clients_connected[k].topics[j].topic, udp_msg.topic) 
								== 0 && clients_connected[k].topics[j].flag == 1) {
								ret = send(clients_connected[k].socket, &udp_msg, 
												sizeof(struct udp_message), 0);
								DIE(ret < 0, "send");
							}				
					}
					//retin pentru fiecare client deconectat mesajele venite 
					for(int k = 0; k < index_disc; k++) {
						int contor = clients_disconnected[k].contor;
						for(int j = 0; j < contor; j++) {
							if(strcmp(clients_disconnected[k].topics[j].topic, udp_msg.topic) 
								== 0 && clients_disconnected[k].topics[j].flag == 1) {
								int contor_msg = clients_disconnected[k].contor_msg;
								clients_disconnected[k].udp_msg[contor_msg] = udp_msg;
								contor_msg++;
								clients_disconnected[k].contor_msg++;
								struct udp_message *temp;
								temp = realloc(clients_disconnected[k].udp_msg, 
									(contor_msg + 1)*sizeof(struct udp_message));
								if(temp == NULL){
							        printf("Cannot allocate more memory.\n");
							        exit(1);
							    }
							    else
									clients_disconnected[k].udp_msg = temp;
								break;

							}	
						}
					}
				} else if (i == tcp_socket) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newtcp_socket = accept(i, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newtcp_socket < 0, "accept");

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newtcp_socket, &read_fds);
					if (newtcp_socket > fdmax) { 
						fdmax = newtcp_socket;
					}

					//dezactivez algoritmul lui Nagle
					int result = setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, 
													(char*) &flag, sizeof(int));
					DIE(result < 0, "setsockopt");
					memset(buffer, 0, BUFLEN);

					//primesc id-ul clientului care s-a conectat
					ret = recv(newtcp_socket, buffer, sizeof(buffer) - 1, 0);
					DIE(ret < 0, "receive");
					int ok = 0;

					//daca clientul care tocmai s-a conectat, apare in lista de 
					//deconectati, atunci ii trimit toate mesajele primite cat timp
					//a fost deconectat
					for(int j = 0; j < index_disc; j++) {
						if(strcmp(clients_disconnected[j].id, buffer) == 0) {
							printf("New client (%s), connected from %s:%d\n",
								clients_disconnected[j].id, inet_ntoa(cli_addr.sin_addr), 
								ntohs(cli_addr.sin_port));
							int contor_msg = clients_disconnected[j].contor_msg;
								
							for(int k = 0; k < contor_msg; k++) {
								struct udp_message msg;
								msg = clients_disconnected[j].udp_msg[k];
								ret = send(clients_disconnected[j].socket, &msg, 
													sizeof(struct udp_message), 0);
								DIE(ret < 0, "send");
							}
							clients_connected[index] = clients_disconnected[j];
							clients_connected[index].socket = newtcp_socket;
							index++;
							struct client_struct *temp;
							temp = realloc(clients_connected, 
									(index + 1)*sizeof(struct client_struct));
							if(temp == NULL)
						    {
						        printf("Cannot allocate more memory.\n");
						        exit(1);
						    }
						    else{
						    	clients_connected = temp;
						    }
							memmove(&clients_disconnected[j], &clients_disconnected[j + 1], 
								(index_disc - j - 1)*sizeof(struct client_struct));
							index_disc--;
							ok = 1;
							break;
						}	
					}
					if(ok == 0) {
						//verific ca id-ul exista deja in lista clientilor deja conectati
						int id_used = 0;
						for(int j = 0; j < index; j++) {
							if(strcmp(clients_connected[j].id, buffer) == 0){
								struct udp_message udp_msg;
								memset(&udp_msg, 0, sizeof(udp_msg));
								strcpy(udp_msg.payload, "id");
								ret = send(newtcp_socket, &udp_msg, sizeof(struct udp_message), 0);
								DIE(ret < 0, "Send failed");
								id_used = 1;
								break;
							}
						}
						//daca id-ul este unic, adaug clientul in lista
						if(id_used == 0) {
							clients_connected[index].topics = malloc(sizeof(struct topic_struct));
							clients_connected[index].socket = newtcp_socket;
							clients_connected[index].contor = 0;
							strcpy((clients_connected[index].id), buffer);
							printf("New client (%s), connected from %s:%d\n",
									clients_connected[index].id, 
							inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
							index++;
							struct client_struct *temp;
							temp = realloc(clients_connected, 
										(index + 1)*sizeof(struct client_struct));
							if(temp == NULL)
						    {
						        printf("Cannot allocate more memory.\n");
						        exit(1);
						    }
						    else{
						    	clients_connected = temp;
						    }
						}
					}

				} else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					struct tcp_message tcp_msg;
					ret = recv(i, &tcp_msg, sizeof(struct tcp_message), 0);
					DIE(ret < 0, "receive");
					if (ret == 0) {
						// se scoate din multimea de citire socketul inchis 
						//il adaug in lista clientilor deconectati
						for(int j = 0; j < index; j++) {
							if(i == clients_connected[j].socket){
								printf("Client (%s) disconnected.\n", clients_connected[j].id);
								clients_disconnected[index_disc] = clients_connected[j];
								clients_disconnected[index_disc].contor_msg = 0;
								clients_disconnected[index_disc].udp_msg = 
											malloc(sizeof(struct udp_message));
								index_disc++;
								struct client_struct *temp;
								temp = realloc(clients_disconnected, 
									(index_disc + 1)*sizeof(struct client_struct));
								if(temp == NULL)
							    {
							        printf("Cannot allocate more memory.\n");
							        exit(1);
							    }
							    else
									clients_disconnected = temp;
								break;
							}
						}
						//il sterg din lista clientilor conectati
						for(int k = 0; k < index; k++) {
							if(i == clients_connected[k].socket) {
								memmove(&clients_connected[k], &clients_connected[k + 1], 
									(index - k - 1)*sizeof(struct client_struct));
								index--;
							}

						}
						FD_CLR(i, &read_fds);
						// conexiunea s-a inchis
						close(i);

					 } else {
					 	for(int j = 0; j < index; j++) {
					 		if(i == clients_connected[j].socket) {
					 			if(tcp_msg.type == 1) {//clientul vrea sa faca subscribe
									int contor = clients_connected[j].contor;
									clients_connected[j].topics[contor].flag = tcp_msg.flag;
									strcpy(clients_connected[j].topics[contor].topic, 
																	tcp_msg.topic);
									contor++;
									clients_connected[j].contor++;
									struct topic_struct *temp;
									temp = realloc(clients_connected[j].topics, 
											(contor + 1)*sizeof(struct topic_struct));
									if(temp == NULL) {
								        printf("Cannot allocate more memory.\n");
								        exit(1);
								    } else{
								    	clients_connected[j].topics = temp;
								    }
								} else if(tcp_msg.type == 0) {//clientul vrea sa faca unsubscribe
									int contor = clients_connected[j].contor;
									for(int k = 0; k < contor; k++) {
										int len = strlen(clients_connected[j].topics[k].topic);
										if(strncmp(clients_connected[j].topics[k].topic, 
														tcp_msg.topic, len) == 0) {
											memmove(&clients_connected[j].topics[k], 
												&clients_connected[j].topics[k+1], 
												(contor - k - 1)*sizeof(struct topic_struct));
											clients_connected[j].contor--;
										}
									}
								}
							}
					 	}
					}
				}
			}
		}
	}
	for(int i = 0; i < index; i++){
		free(clients_connected[i].topics);
	}
	free(clients_connected);
	for(int i = 0; i < index_disc; i++){
		free(clients_disconnected[i].topics);
		free(clients_disconnected[i].udp_msg);
	}
	free(clients_disconnected);
	close(tcp_socket);
	close(udp_socket);
	return 0;
}
