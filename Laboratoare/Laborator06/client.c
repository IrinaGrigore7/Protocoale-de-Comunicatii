/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	client mini-server de backup fisiere
*/

#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "helpers.h"

void usage(char*file)
{
	fprintf(stderr,"Usage: %s ip_server port_server file\n",file);
	exit(0);
}

/*
*	Utilizare: ./client ip_server port_server nume_fisier_trimis
*/
int main(int argc,char**argv)
{
	if (argc!=4)
		usage(argv[0]);
	
	int fd;
	struct sockaddr_in to_station;
	char buf[BUFLEN];


	/*Deschidere socket*/
	
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0)
    	perror("socket creation failed");

	
	/* Deschidere fisier pentru citire */
	DIE((fd=open(argv[3],O_RDONLY))==-1,"open file");
	
	/*Setare struct sockaddr_in pentru a specifica unde trimit datele*/
	
	to_station.sin_family = AF_INET;
	to_station.sin_port = htons(12345);

	
	inet_aton("127.0.0.1", &to_station.sin_addr);
	int n;

	while((n = read(fd, buf, BUFLEN)) > 0){
		int ss = sendto(s, buf, n, 0, (struct sockaddr*) &to_station, sizeof(to_station));
		if(ss < 0){
			perror("Sendto error");
			break;
		}
	}

	
	/*
	*  cat_timp  mai_pot_citi
	*		citeste din fisier
	*		trimite pe socket
	*/	


	/*Inchidere socket*/
	int c = close(s);
	if(c < 0){
	 	perror("Close faile");
	 }
	close(fd);
	
	/*Inchidere fisier*/

	return 0;
}
