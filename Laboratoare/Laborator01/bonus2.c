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
    	char buf[1024];
    	int offset;
    	int i;

    	if(fd < 0)
    		fatal("nu pot deschide fisierul");

    	offset = lseek(fd, 0, SEEK_END);
    	lseek(fd, -1, SEEK_END);
    	
    	while(offset != 0){
    		read(fd, buf, 1);

    		i = 0;
    		if(buf[0] != '\n'){
    			
    			i++;
    			offset--;
    		}

    		//write(1, buf, 1);
    	}
    	
 		close(fd);
    	return 0;
    }
