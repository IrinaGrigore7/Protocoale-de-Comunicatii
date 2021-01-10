#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000


int main(int argc,char** argv){
  init(HOST,PORT);
  msg t;
  //========numele fisierului======
  int fd = open("file", O_RDONLY);
  if(fd < 0)
        printf("nu pot deschide fisierul\n");

  //Send dummy message:
  printf("[send] Sending name file...\n");
  sprintf(t.payload,"%s", "file");
  t.len = strlen(t.payload)+1;
  send_message(&t);
  
  // Check response:
  if (recv_message(&t)<0){
    perror("Receive error ...");
    return -1;
  }
  else {
    printf("[send] Got reply with payload: %s\n", t.payload);
  }



  //==========dimensiunea fisierului======
  printf("[send] Sending size file...\n");
  int size = lseek(fd, 0, SEEK_END);
  sprintf(t.payload,"%d",size );
  t.len = strlen(t.payload)+1;
  send_message(&t);
  
  // Check response:
  if (recv_message(&t)<0){
    perror("Receive error ...");
    return -1;
  }
  else {
    printf("[send] Got reply with payload: %s\n", t.payload);
  }
  

  //======citire din fisier======
  lseek(fd, 0, SEEK_SET);
  while(read(fd, t.payload, 1400)){
    
    printf("[send] Sending file...\n");
    //read(fd, t.payload, 1400);
    
    t.len = strlen(t.payload);
    send_message(&t);

    if (recv_message(&t)<0){
      perror("Receive error ...");
      return -1;
    }
    else {
      printf("[send] Got reply with payload: %s\n", t.payload);
    }
  
  }
  close(fd);





  return 0;
}
