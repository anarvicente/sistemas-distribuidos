#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <unistd.h>

#define h_addr h_addr_list[0] /* for backward compatibility */

struct sockaddr_in serv_addr;
pthread_t t_read, t_write; /* para cancelar a thread oposta */
char nick[270];

void error(char *msg){
    perror(msg);
    exit(0);
}

void* write_socket(void* sockfd){
    char buffer[256], nickname[50];
    int n, msg=1;
    strcpy(nickname, nick); /* p/ guardar o nome escolhido */

    while(msg) {
        bzero(buffer,256);
        fgets(buffer,255,stdin);
        strcat(nick, ":");
        strcat(nick, buffer);
        n = write(*(int*)sockfd,nick,strlen(nick));
        if (n < 0) error("ERROR writing to socket");
        buffer[strcspn(buffer, "\n")] = 0;
        msg = strcmp(buffer, "bye");
        bzero(nick, 50); /* limpar o buffer */
        strcat(nick, nickname);

    }

    pthread_cancel(t_read);
    return NULL;
}

void* read_socket(void* sockfd){
    char buffer[256];
    int n,msg=1;

    while(msg){
        bzero(buffer,256);
        n = read(*(int*)sockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        printf("%s\n", buffer);
        buffer[strcspn(buffer, "\n")] = 0;
        msg = strcmp(buffer,"bye");
    }

    pthread_cancel(t_write);
    return NULL;
}

int socket_create(int argc, char *argv[]){
    int sockfd, portno;
    struct hostent *server;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }else if (argc == 3){
        strcpy(nick, "Fulano");
    }else{
        strcpy(nick, argv[3]);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    return sockfd;

}

void make_connect(int sockfd){

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        error("ERROR connecting");
    }

}

int main(int argc, char *argv[])
{
    int sockfd;

    sockfd = socket_create(argc, argv);

    make_connect(sockfd);

    /* parte threads */
    pthread_create(&t_write, NULL, write_socket, &sockfd);
    pthread_create(&t_read, NULL, read_socket, &sockfd);

    pthread_join(t_write, NULL);
    pthread_join(t_read, NULL);

    close(sockfd);  /* liberar o socket */
    return 0;
}
