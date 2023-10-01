/*
  Code of client's part
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

#define MAX_BUFFER 1024

int tcp_socket;


void finish(int fd) {
	close(tcp_socket);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	signal(SIGINT, finish);

	struct sockaddr_in my_addr;
	int nbytes = 0;
	char buffer[MAX_BUFFER];

	if (argc != 4) {
		fprintf(stderr, " $ ./client [cliente_id] [ip] [port] \n");
		exit(EXIT_FAILURE);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr(argv[2]);
	my_addr.sin_port = htons(atoi(argv[3]));

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		fprintf(stderr, "Error in socket creation\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Socket successfully created...\n");
	}

	if (connect(tcp_socket, (struct sockaddr *)&my_addr,
		    sizeof(my_addr)) == -1) {
		fprintf(stderr, "Error in connect\n");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	} else {
		printf("connected to the server...\n");
	}

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "Hello server! From client: ");
	strcat(buffer, argv[1]);
	strcat(buffer, "\n");
	printf("> %s", buffer);
	if (send(tcp_socket, buffer, sizeof(buffer), 0) == -1) {
		fprintf(stderr, "Error in send\n");
		return -1;
	}

	memset(buffer, 0, sizeof(buffer));
	nbytes = recv(tcp_socket, (void *)buffer, sizeof(buffer), 0);
	if (nbytes == -1) {
		fprintf(stderr, "Error in recv\n");
		return -1;
	}

	buffer[nbytes] = '\0';
	printf("+++ %s", buffer);

	close(tcp_socket);
	exit(EXIT_SUCCESS);
}
