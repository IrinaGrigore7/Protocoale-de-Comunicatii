
CFLAGS = -Wall -g

# Portul pe care asculta serverul (de completat)
PORT = 1234

# Adresa IP a serverului (de completat)
IP_SERVER = 127.0.0.1

# ID-ul clientului
ID_CLIENT = 
build: server client

# Compileaza server.c
server: server.c

# Compileaza client.c
client: client.c -lm

.PHONY: clean run_server run_client

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul
run_client:
	./client ${ID_CLIENT} ${IP_SERVER} ${PORT}

clean:
	rm -f server client
