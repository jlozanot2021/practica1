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
#include <pthread.h>

#define LISTEN_BACKLOG 100

int array_sockets[100];
int tcp_socket;
int threads_actives = 0;

void
finish_thread(int fd){
    pthread_exit(NULL);

    exit(EXIT_SUCCESS);
}

void
finish(int i){
    for(i = 0; i < LISTEN_BACKLOG; i++)
        close(array_sockets[i]);
    close(tcp_socket);
    exit(EXIT_SUCCESS);
}

void *
thread_client(void *thread){
    int nbytes = 0, *sockfd;
    char buffer[100];

    sockfd = ((int *)thread);

    memset(buffer, 0, sizeof(buffer));

    nbytes = recv(*sockfd, (void*) buffer, sizeof(buffer), 0);
    if (nbytes == -1){
        fprintf(stderr, "Error in recv\n");
        exit(EXIT_FAILURE);
    }

    buffer[nbytes] = '\0'; 
    printf("+++ %s", buffer);

    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer , "Hello client!\n");
    usleep((rand() % (1500000)) + 500000);
    printf("> %s", buffer);
    if (send(*sockfd, buffer, sizeof(buffer), 0) == -1){
        fprintf(stderr, "Error in send\n");
        exit(EXIT_FAILURE);
    }

    while(1){
        signal(SIGINT,finish_thread);
        nbytes = recv(*sockfd, (void*) buffer, sizeof(buffer), 0);
        if (nbytes == -1){
            fprintf(stderr, "Error in recv\n");
            close(*sockfd);
            free(sockfd);
            exit(EXIT_FAILURE);
        }
        if( nbytes == 0 ){
            close(*sockfd);
            free(sockfd);
            fprintf(stderr,"borrar\n");
            threads_actives--;
            pthread_exit(NULL);
        }
        buffer[nbytes] = '\0'; 
        printf("+++ %s", buffer);
        printf("> ");
        memset(buffer, 0, sizeof(buffer));
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (send(*sockfd, buffer, sizeof(buffer),0) == -1){
                fprintf(stderr, "Error in send\n");
                close(*sockfd);
                exit(EXIT_FAILURE);
            }
        }
    }
}

int 
main(int argc, char **argv)
{
    setbuf(stdout, NULL);

    fd_set readmask;
    struct timeval timeout;
    struct sockaddr_in my_addr, peer_addr;
    socklen_t peer_addr_size;
    int i, *sockfd, nbytes = 0;
    const int enable = 1;
    char buffer[100];
    pthread_t thread;

    if (argc != 2){
        fprintf( stderr , " $ ./server [port] \n");
        exit( EXIT_FAILURE );
    }

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if( tcp_socket < 0){
        fprintf(stderr, "Error in socket creation\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Socket successfully created...\n");
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    my_addr.sin_port = htons(atoi(argv[1]));

    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    }
    if (bind(tcp_socket, (struct sockaddr *) &my_addr,
                   sizeof(my_addr)) == -1){
        fprintf(stderr, "Error in bind\n");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    } else {
        printf("Socket successfully binded...\n");
    }

    if (listen(tcp_socket, LISTEN_BACKLOG) == -1){
        fprintf(stderr, "Error in listen\n");
        close(tcp_socket);
        exit(EXIT_FAILURE);
    } else {
        printf("Server listening...\n");
    }
    peer_addr_size = sizeof(struct sockaddr_in);
    while(1){
        if(threads_actives >= 100){
            continue;
        }
        sockfd = malloc(sizeof(int));
        *sockfd = accept(tcp_socket, (struct sockaddr *) &peer_addr, &peer_addr_size);
        if (*sockfd == -1){
            fprintf(stderr, "Error in accept\n");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }
        pthread_create(&thread, NULL, &thread_client, (void *)sockfd);
        threads_actives++;
    }
    signal(SIGINT,finish);
    close(tcp_socket);
    close(*sockfd);
    free(sockfd);
    exit(EXIT_SUCCESS);
}
