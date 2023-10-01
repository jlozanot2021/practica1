/*
  Code of server's part
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>

#define MAX_BUFFER 1024
#define PORT 8080
#define LISTEN_BACKLOG 50

int tcp_socket, sockfd;

void finish(int fd) {
	close(tcp_socket);
	close(sockfd);
	exit(EXIT_SUCCESS);
}

int start() {
	int nbytes = 0;
	char buffer[MAX_BUFFER];

	memset(buffer, 0, sizeof(buffer));

	nbytes = recv(sockfd, (void *)buffer, sizeof(buffer), 0);
	if (nbytes == -1) {
		fprintf(stderr, "Error in recv\n");
		return -1;
	}

	buffer[nbytes] = '\0';
	printf("+++ %s", buffer);

	memset(buffer, 0, sizeof(buffer));
	strcpy(buffer, "Hello client!\n");
	printf("> %s", buffer);
	if (send(sockfd, buffer, sizeof(buffer), 0) == -1) {
		fprintf(stderr, "Error in send\n");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	signal(SIGINT, finish);
	fd_set readmask;
	struct timeval timeout;
	struct sockaddr_in my_addr, peer_addr;
	socklen_t peer_addr_size;
	int select_n, nbytes = 0;
	const int enable = 1;
	char buffer[MAX_BUFFER];

	tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket < 0) {
		fprintf(stderr, "Error in socket creation\n");
		exit(EXIT_FAILURE);
	} else {
		printf("Socket successfully created...\n");
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	my_addr.sin_port = htons(PORT);

	if (setsockopt
	    (tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	}
	if (bind(tcp_socket, (struct sockaddr *)&my_addr,
		 sizeof(my_addr)) == -1) {
		fprintf(stderr, "Error in bind\n");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	} else {
		printf("Socket successfully binded...\n");
	}

	if (listen(tcp_socket, LISTEN_BACKLOG) == -1) {
		fprintf(stderr, "Error in lsiten\n");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	} else {
		printf("Server listening...\n");
	}

	peer_addr_size = sizeof(struct sockaddr_in);
	sockfd =
	    accept(tcp_socket, (struct sockaddr *)&peer_addr, &peer_addr_size);
	if (sockfd == -1) {
		fprintf(stderr, "Error in listen\n");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	}

	if (start() == -1) {
		close(tcp_socket);
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	memset(buffer, 0, sizeof(buffer));
	while (1) {

		FD_ZERO(&readmask);	// Reset la mascara
		FD_SET(sockfd, &readmask);	// Asignamos el nuevo descriptor
		FD_SET(STDIN_FILENO, &readmask);	// Entrada
		timeout.tv_sec = 0;
		timeout.tv_usec = 500000;	// Timeout de 0.5 seg.
		select_n = select(sockfd + 1, &readmask, NULL, NULL, &timeout);
		if (select_n == -1)
			exit(-1);
		if (FD_ISSET(sockfd, &readmask)) {
			// Hay datos que leer del descriptor        
			nbytes =
			    recv(sockfd, (void *)buffer, sizeof(buffer),
				 MSG_DONTWAIT);
			if (nbytes == -1) {
				fprintf(stderr, "Error in recv\n");
				close(tcp_socket);
				close(sockfd);
				exit(EXIT_FAILURE);
			}
			printf("+++ %s", buffer);
		}

		printf("> ");
		fgets(buffer, sizeof(buffer), stdin);
		if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
			fprintf(stderr, "Error in send\n");
			close(tcp_socket);
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		memset(buffer, 0, sizeof(buffer));
	}

	close(tcp_socket);
	close(sockfd);
	exit(EXIT_SUCCESS);
}
