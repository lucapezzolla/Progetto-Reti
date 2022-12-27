#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "functions.h"




struct Pacchetto{
char funzione[25];
char porta[6];
char parametri[20];
char descrizione[50];
} pacchetto[100];


char buffer[10];


int main(int argc, char** argv) {


struct sockaddr_in server;
ssize_t var;
int index = 0;

int sockfd, accfd;

sockfd = Socket(AF_INET,SOCK_STREAM,0);

server.sin_family = AF_INET;
server.sin_addr.s_addr = htonl(INADDR_ANY);
server.sin_port = htons(52000);

Bind(sockfd,(struct sockaddr*)&server,sizeof(server));

Listen(sockfd,1024);

fd_set set;
int fd_disponibili, fd_connectedClient[32]={0}, i;
int maxfd = sockfd;

FD_ZERO(&set);


while(1) {

FD_SET(sockfd,&set);

for(i = 0; i <= maxfd; i++) {
  if(fd_connectedClient[i])
  FD_SET(i,&set);
}


fd_disponibili = select(maxfd+1,&set,NULL,NULL,NULL);
sleep(2);


if(FD_ISSET(sockfd, &set)){
printf("Nuovo client connesso");
accfd = accept(sockfd,(struct sockaddr*)NULL, NULL);
printf(" sul FD %d\n", accfd);
fd_connectedClient[accfd] = 1;//LO METTO NELLA LISTA DEI CLIENT CONNESSI
if(maxfd<accfd)
 maxfd=accfd;//RICONTROLLO IL MAX
 fd_disponibili--;
}

i = 0;

while(fd_disponibili > 0 && i < FD_SETSIZE){
i++;

if(fd_connectedClient[i] == 0){
    continue;
}



if(FD_ISSET(i,&set)) {
 printf("Servo il client connesso sulla fd %d\n...\n", i);
    fd_disponibili--;


char buffer[10];
//IL SERVER LEGGE IL NUMERO DI PORTA DEL CLIENT CHE SI CONNETTE E LO SALVA IN BUFFER
read(accfd,buffer,sizeof(buffer));
printf("Ho letto la porta %s\n",buffer);
int porta = atoi(buffer);      //LA PORTA DEL CLIENT VIENE SALVATA COME INTERO CONVERTENDOLA CON ATOI
write(accfd,"Ho letto",strlen("Ho letto"));   //INVIO CONFERMA AL CLIENT CHE ABBIAMO LETTO LA PORTA
var = read(accfd,&pacchetto[index],sizeof(pacchetto));   //FACCIO LA READ DEL PACCHETTO INVIATOMI DAL CLIENT E METTO IL TUTTO NELLA STRUCT PACCHETTO[INDEX]
index += var/(sizeof(pacchetto[0]));  //SCOSTAMENTO


//CICLO FOR CHE SCORRE TUTTI I PACCHETTI. MI SERVE PER INVIARE AL CLIENT CONNESSO TUTTI I PACCHETTI DEGLI ALTRI CLIENT (FUNZIONI,PORTE ECC...)
for(int j = 0; j < 100; j++) {
//SE IL PACCHETTO J-SIMO HA PORTA UGUALE A BUFFER, CIOE' ALLA PORTA DEL CLIENT CONNESSO, NON INVIARE IL PACCHETTO. IDEM SE IL PACCHETTO J-SIMO HA PORTA UGUALE ALLA STRINGA VUOTA. IN QUESTO CASO SIGNIFICA CHE LA SUA STRUCT NON E' STATA RIEMPITA (NESSUN CLIENT HA FATTO LA WRITE)
if(strcmp(pacchetto[j].porta,buffer) != 0 && strcmp(pacchetto[j].porta,"") != 0)  //ALTRIMENTI INVIA
write(accfd,&pacchetto[j],sizeof(pacchetto[0])*index);
}


close(i);

fd_connectedClient[i] = 0;
printf("Client connesso sulla fd %d servito.\n", i);

if(maxfd == i){    //Se ho raggiunto (e servito) il maxfd devo trovare il nuovo maxfd procedendo a ritroso
    while(fd_connectedClient[--i] ==0) {
        maxfd = i;
        break;
}

}
}
}
}

return 0;
}
