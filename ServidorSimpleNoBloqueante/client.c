/*
  Code of client's part
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

#define PORT 8080

int tcp_socket;

void
finish(int fd){
    close(tcp_socket);
    exit(EXIT_SUCCESS);
}

int
start(int sockfd){
    int nbytes = 0;
    char buffer[100];

    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer , "Hello server!\n");
    printf("> %s", buffer);
    if (send(sockfd, buffer, sizeof(buffer), 0) == -1){
        fprintf(stderr, "Error in send\n");
        return -1;
    }

    memset(buffer, 0, sizeof(buffer));
    nbytes = recv(sockfd, (void*) buffer, sizeof(buffer), 0);
    if (nbytes == -1){
        fprintf(stderr, "Error in recv\n");
        return -1;
    }

    buffer[nbytes] = '\0'; 
    printf("+++ %s", buffer);

    return 0;
}

int 
main(int argc, char **argv)
{
    setbuf(stdout, NULL);
    signal(SIGINT,finish);
    fd_set readmask;
    struct timeval timeout;
    struct sockaddr_in my_addr;
    int tcp_socket, select_n = 0, nbytes = 0;
    char buffer[100];

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(PORT);

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if( tcp_socket < 0){
        fprintf(stderr, "Error in socket creation\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Socket successfully created...\n");
    }

    if (connect(tcp_socket, (struct sockaddr*)&my_addr,
                   sizeof(my_addr)) == -1){
        fprintf(stderr, "Error in connect\n");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    } else {
        printf("connected to the server...\n");
    }

    if (start(tcp_socket) == -1){
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }
    memset(buffer, 0, sizeof(buffer));
    while(1){
        
        FD_ZERO(&readmask); // Reset la mascara
        FD_SET(tcp_socket, &readmask); // Asignamos el nuevo descriptor
        FD_SET(STDIN_FILENO, &readmask); // Entrada
        timeout.tv_sec=0; timeout.tv_usec=500000; // Timeout de 0.5 seg.
        select_n = select(tcp_socket+1, &readmask, NULL, NULL, &timeout);
        if (select_n ==-1)
            exit(-1);  
        if (FD_ISSET(tcp_socket, &readmask)){      
            nbytes = recv(tcp_socket, (void*) buffer, sizeof(buffer), MSG_DONTWAIT);
            if (nbytes == -1){
                fprintf(stderr, "Error in recv\n");
                close(tcp_socket);
                exit(EXIT_FAILURE);
            }
        }
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);
        if (send(tcp_socket, buffer, sizeof(buffer), 0) == -1){
            fprintf(stderr, "Error in send\n");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }  
        memset(buffer, 0, sizeof(buffer));
    }

    close(tcp_socket);
    exit(EXIT_SUCCESS);
}
