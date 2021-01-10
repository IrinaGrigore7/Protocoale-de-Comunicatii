#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

int main(int argc, char *argv[])
{
	msg t;
	int i, res;
	int w;
	
	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);

	/* printf("[SENDER]: BDP=%d\n", atoi(argv[1])); */
	w = (atoi(argv[1])*1000) / (sizeof(msg)*8);
	printf("%d\n", w);
	
	for (i = 0; i < w; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
	}
		for (i = 0; i < COUNT-w; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}

		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	
	}

	for (i = 0; i < w; i++) {
		/* cleanup msg */
		//memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		//t.len = MSGSIZE;
		
		
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}
	

	printf("[SENDER] Job done, all sent.\n");
		
	return 0;
}
