#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001


int main(int argc,char** argv){
  msg r;
  init(HOST,PORT);
  int fis = open("fis", O_WRONLY);
  


  if (recv_message(&r)<0){
    perror("Receive message");
    return -1;
  }
  printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);


  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  send_message(&r);
  printf("[recv] ACK sent\n");



  // Send ACK:
    	if (recv_message(&r)<0){
    perror("Receive message");
    return -1;
  }
  printf("[recv] Got msg with payload: <%s>, sending ACK...alalal\n", r.payload);
   
  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
 
  send_message(&r);
  printf("[recv] ACK sent\n");





  while(recv_message(&r)>= 0){
  	write(fis, r.payload, r.len);
  	send_message(&r);
  }

 	if (recv_message(&r)<0){
    perror("Receive message");
    return -1;
 	 }

  printf("[recv] Got msg with payload: <%s>, sending ACK...\n", r.payload);
  write(fis, r.payload, 1400);
  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  //size = size - r.len;
  send_message(&r);
  printf("[recv] ACK sent\n");
  
//}

close(fis);


  return 0;
}
