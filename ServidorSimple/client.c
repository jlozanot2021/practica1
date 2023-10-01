/*
  Code of client's part
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define MAX_BUFFER 1024
#define PORT 8080

int tcp_socket;

void finish(int fd) {
	close(tcp_socket);
	exit(EXIT_SUCCESS);
}

int start(int sockfd) {
	int nbytes = 0;
	char buffer[MAX_BUFFER];

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "Hello server!");
	printf("> %s\n", buffer);
	if (send(sockfd, buffer, sizeof(buffer), 0) == -1) {
		fprintf(stderr, "Error in send\n");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	signal(SIGINT, finish);
	struct sockaddr_in my_addr;
	int nbytes = 0;
	char buffer[MAX_BUFFER];

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(PORT);

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

	if (start(tcp_socket) == -1) {
		close(tcp_socket);
		exit(EXIT_FAILURE);
	}

	while (tcp_socket >= 0) {

		nbytes = recv(tcp_socket, (void *)buffer, sizeof(buffer), 0);
		if (nbytes == -1) {
			fprintf(stderr, "Error in recv\n");
			close(tcp_socket);
			exit(EXIT_FAILURE);
		}
		if (nbytes == 0) {
			fprintf(stderr, "Connection not found\n");
			close(tcp_socket);
			exit(EXIT_FAILURE);
		}
		buffer[nbytes] = '\0';
		printf("+++ %s", buffer);
		printf("> ");
		memset(buffer, 0, sizeof(buffer));
		if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
			nbytes = send(tcp_socket, buffer, sizeof(buffer), 0);
			if (nbytes == -1) {
				fprintf(stderr, "Error in send\n");
				close(tcp_socket);
				exit(EXIT_FAILURE);
			}
		}
	}

	close(tcp_socket);
	exit(EXIT_SUCCESS);
}
