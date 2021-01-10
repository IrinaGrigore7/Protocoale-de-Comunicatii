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
    	int copiat;

    	if(fd < 0)
    		fatal("nu pot deschide fisierul");

    	lseek(fd, 0, SEEK_SET);
 
   	 while ((copiat = read(fd, buf, sizeof(buf)))) {
       	 if (copiat < 0)
            fatal("Eroare la citire");
        	write(1, buf, copiat);
        if (copiat < 0)
            fatal("Eroare la scriere");
    }
    close(fd);
    	return 0;
    }

