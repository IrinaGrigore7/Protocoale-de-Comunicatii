#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* pentru open(), exit() */
#include <fcntl.h> /* O_RDWR */
#include <errno.h> /* perror() */
 
void fatal(char * mesaj_eroare)
{
    perror(mesaj_eroare);
    exit(0);
}

    int main(){
    	int fd = open("file.txt", O_RDONLY);
        int file = open("file.txt", O_RDONLY);
    	char buf[1024];
        
        int idx = 0;

    	if(fd < 0)
    		fatal("nu pot deschide fisierul");

        lseek(fd, -1, SEEK_END);
        while(lseek(fd, -1, SEEK_CUR) != -1){
            read(fd, buf, 1);
             lseek(fd, -1, SEEK_CUR);
             idx++;
             if(buf[0] != '\n'){
                
                lseek(fd, 1, SEEK_CUR);  
   	            read(fd, buf, idx);
                write(1, buf, idx);
              

                lseek(fd, -idx-1, SEEK_CUR);
                idx = 0;
            }
        }
        read(fd, buf, idx+1);
        write(1, buf, idx+1);
        close(fd);
    	return 0;
    }
