CC=gcc
CFLAGS=-I -Wall -Wshadow
DEPS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
all: client server
client: client.o
	$(CC) -o client client.o $(CFLAGS)
		
server: server.o
	$(CC) -o server server.o $(CFLAGS)




clean:
	rm -f *.o
