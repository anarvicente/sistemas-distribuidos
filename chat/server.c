/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>


#define QTD_CLIENT 5

int clientsock[QTD_CLIENT];

pthread_t thread[QTD_CLIENT];
pthread_mutex_t lockwrite;
struct sockaddr_in serv_addr, cli_addr;



void error(char *msg)
{
    perror(msg);
    exit(1);
}

/* Funcao que recebe a mensagem do cliente e remove o nickname
   e retorna apenas a mensagem */
char* remove_nickname(char *nick){
	int final, flag=1, i=0;
	while(flag){
		if(nick[i] == ':'){
			flag=0;
		}
		i++;
	}
	final = strlen(nick) - i;
	char mensagem[final];

  memcpy(mensagem, &nick[i], final);
	bzero(nick, 256);
	strcpy(nick, mensagem);
	nick[strcspn(nick, "\n")] = 0;
	return nick;
}

/* Funcao responsavel por encaminhar as mensagens recebidas para todos os
   clientes. */
void* read_socket(void* newsockfd){ /* espera void*  */
      char buffer[256], msg[256], nickname[256];
      int n, i, j;

      while(1){
        bzero(buffer,256);
        n = read(*(int*)newsockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");

        strcpy(msg, buffer);

        remove_nickname(msg);

        if(strcmp(msg,"bye") == 0){
          for(i=0; i < QTD_CLIENT; i++){
            if(*(int*)newsockfd == clientsock[i]){
              clientsock[i] = -1;
              j=i;
              /* Necessario para enviar mensagem ao restante dos clientes ainda conectados
                 antes da thread do cliente corrente morrer. */
              while(j < QTD_CLIENT){
                if(clientsock[j] != -1){
                  strcpy(nickname, buffer);
                  nickname[strcspn(nickname, ":")] = 0;
                  strcat(nickname, " saiu..");
                  n = write(clientsock[j],nickname,strlen(nickname));
                }
                j++;
              }
              pthread_exit((void*)thread[i]); /* Nao tava cancelando na hora com o cancel */
              /* Eu acho que eh pq a thread morre, dai nao termina o loop
                 era isso mesmo */

            }else if(clientsock[i] != -1){
              strcpy(nickname, buffer);
              nickname[strcspn(nickname, ":")] = 0;
              strcat(nickname, " saiu..");
              n = write(clientsock[i],nickname,strlen(nickname));
            }

          }
        }

        for(i=0; i < QTD_CLIENT; i++){
          pthread_mutex_lock(&lockwrite);
          if((clientsock[i] != -1) && (clientsock[i] != *(int*)newsockfd)){
            n = write(clientsock[i],buffer,strlen(buffer));
          }
          pthread_mutex_unlock(&lockwrite);
        }

      }
      return NULL;
}


int socket_create(int argc){
     int sockfd;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0)
        error("ERROR opening socket");

     return sockfd;

}

void make_bind(int sockfd, char *argv[]){
  int portno;

  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr,
      sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

}

void make_listen(int sockfd){
    listen(sockfd,5);
}

void make_accept(int sockfd){
    socklen_t clilen;
    int i=0;
    clilen = sizeof(cli_addr);

    /* VER UMA MANEIRA DE FICAR ALWAYS ON - SO FALTA ISSO!! */
    while( i < QTD_CLIENT){
      clientsock[i] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (clientsock[i] < 0){
          error("ERROR on accept");
        }

        pthread_create(&thread[i], NULL, read_socket, &clientsock[i]);
        i++;

      }
  }

int main(int argc, char *argv[])
{
    int sockfd, i;

    for(i=0; i < QTD_CLIENT; i++){
        clientsock[i] = -1;
    }

    sockfd = socket_create(argc);
    make_bind(sockfd,argv);
    make_listen(sockfd);

    printf("Esperando alguém conectar..\n");

    make_accept(sockfd);

    for(i=0; i<QTD_CLIENT; i++){
      pthread_join(thread[i], NULL);
    }

    printf("O bate-papo acabou..\n");
    close(sockfd);
    return 0;
}
