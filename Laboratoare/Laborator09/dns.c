// Protocoale de comunicatii
// Laborator 9 - DNS
// dns.c

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int usage(char* name)
{
	printf("Usage:\n\t%s -n <NAME>\n\t%s -a <IP>\n", name, name);
	return 1;
}

// Receives a name and prints IP addresses
void get_ip(char* name)
{
	int ret;
	struct addrinfo hints, *result, *p;
	char buf[100];
	// TODO: set hints
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;	
	// TODO: get addresses
	ret = getaddrinfo("google.com", NULL, &hints, &result);
	if(ret < 0) {
		printf("%s\n", gai_strerror(ret));
	}
	while(result->ai_next != NULL) {
		p = result;
		if (p->ai_family == AF_INET) {
    		struct sockaddr_in* in = (struct sockaddr_in*) p->ai_addr;
    		inet_ntop(p->ai_family, &in->sin_addr, buf, 100);
    		printf ("IP = %s\n", buf);
    		printf("port %d\n", ntohs(in->sin_port));
    		if(p->ai_canonname != NULL){
    			printf ("canonname: %s\n", p->ai_canonname);
    		}
    		printf("socktype: %d protocol: %d\n", p->ai_socktype, p->ai_protocol);
    		printf("\n");


		} else if (p->ai_family == AF_INET6) {
   			struct sockaddr_in6* in6 = (struct sockaddr_in6*) p->ai_addr;
    		inet_ntop(p->ai_family, &in6->sin6_addr, buf, 100);
    		printf ("IP = %s\n", buf);
    		printf("port %d\n", ntohs(in6->sin6_port));
    		if(p->ai_canonname != NULL){
    			printf ("canonname: %s\n", p->ai_canonname);
    		}
    		printf("socktype: %d protocol: %d\n", p->ai_socktype, p->ai_protocol);
    		printf("\n");
		}
		result = result->ai_next;

	}
	freeaddrinfo(result);
	// TODO: iterate through addresses and print them

	// TODO: free allocated data
}

// Receives an address and prints the associated name and service
void get_name(char* ip)
{
	int ret;
	struct sockaddr_in addr;
	char host[1024];
	char service[20];
	// TODO: fill in address data
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	inet_aton(ip, &addr.sin_addr);
	// TODO: get name and service
	ret = getnameinfo((struct sockaddr*) &addr, sizeof(addr), host, 100, service, 100, 0);
	if(ret < 0) {
		printf("%s\n", gai_strerror(ret));
	}
	printf("name: %s\n", host);
	printf("service %s\n", service);
	// TODO: print name and service
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		return usage(argv[0]);
	}

	if (strncmp(argv[1], "-n", 2) == 0) {
		get_ip(argv[2]);
	} else if (strncmp(argv[1], "-a", 2) == 0) {
		get_name(argv[2]);
	} else {
		return usage(argv[0]);
	}

	return 0;
}
