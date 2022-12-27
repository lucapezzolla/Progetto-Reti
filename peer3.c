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
} pacchetto[3],ricezione[100],richiesta;


int moltiplicazione(int a, int b) {
return a*b;
}

float divisione(float a, float b) {
if(a >= b)
return a/b;
else
return 0.0;
}

char* frase(char *stringa) {
char* stringa2 = "Ma che bel progettino";
return strcat(stringa2,stringa);
}




int main (int argc, char **argv) {


int sockfd;
struct sockaddr_in client, client2;  //LA PRIMA STRUCT SERVE PER CONNETTERMI AL SERVER PRINCIPALE. LA SECONDA STRUTTURA SERVIRA' PER AGIRE DA CLIENT VERSO UN PEER
int index = 0;
ssize_t var;

 if(argc < 2){
    perror("Hai dimenticato di inserire gli argomenti!");
 }




strcpy(pacchetto[0].funzione,"moltiplicazione");
strcpy(pacchetto[0].porta,argv[1]);
strcpy(pacchetto[0].parametri,"int int");
strcpy(pacchetto[0].descrizione,"Funzione che effettua la moltiplicazione");
strcpy(pacchetto[1].funzione,"divisione");
strcpy(pacchetto[1].porta,argv[1]);
strcpy(pacchetto[1].parametri,"float float");
strcpy(pacchetto[1].descrizione,"Funzione che effettua la divisione");
strcpy(pacchetto[2].funzione,"frase");
strcpy(pacchetto[2].porta,argv[1]);
strcpy(pacchetto[2].parametri,"char*");
strcpy(pacchetto[2].descrizione,"Funzione che concatena due stringhe");


sockfd = Socket(AF_INET,SOCK_STREAM,0);

client.sin_port = htons(52000);   //PORTA SERVER;
client.sin_family = AF_INET;


if(inet_pton(AF_INET,"127.0.0.1",&client.sin_addr) <= 0) {
  perror("Inet error");
  exit(0);
}


Connect(sockfd,(struct sockaddr*)&client,sizeof(client));

printf("Benvenuto nel programma di gestione dei peer\n");
write(sockfd,&pacchetto[0].porta,sizeof(pacchetto[0].porta));  //INVIO AL SERVER IL MIO N. DI PORTA COSI' CHE LUI POSSA SALVARSELO
char buffer[10];
read(sockfd,buffer,sizeof(buffer));  //MI SALVO LA RISPOSTA DEL SERVER IN UN GENERICO BUFFER. QUESTA READ MI DICE SOLO CHE IL SERVER HA LETTO LA MIA PORTA
write(sockfd,&pacchetto,sizeof(pacchetto));  //ORA INVIO LA MIA STRUCT PACCHETTO AL SERVER


var = read(sockfd,&ricezione[index],sizeof(ricezione));  //LEGGO LE STRUCT PACCHETTI CHE IL SERVER MI INVIA E LE METTO NELLA STRUCT RICEZIONE, NELL'APPOSITO INDEX

index += var/(sizeof(ricezione[0]));  //SCOSTAMENTO

//CICLO FOR PER STAMPARE I CORRISPETTIVI PACCHETTI CHE IL SERVER MI HA INVIATO
for(int i = 0; i < 100; i++) {
//SE LA PORTA DELLA STRUCT RICEZIONE E' UGUALE ALLA MIA, NON STAMPO (NON HA SENSO STAMPARE LA MIA STRUCT). IDEM SE L'I-SIMA PORTA E' UGUALE ALLA STRINGA VUOTA.
if(strcmp(ricezione[i].porta,argv[1]) != 0 && strcmp(ricezione[i].porta,"") != 0)
printf("\n%d\n%s\n%s\n%s\n%s\n",i,ricezione[i].funzione,ricezione[i].porta,ricezione[i].parametri,ricezione[i].descrizione);  //ALTRIMENTI STAMPO LE INFORMAZIONI CON L'INDICE DELLA STRUCT RICEZIONE DOVE SONO CONTENUTE
}



//close(sockfd);


//INIZIALIZZO LA STRUTTURA DEL PEER IN MODO TALE CHE POSSA AGIRE DA SERVER

struct sockaddr_in server;
int accfd;
int sockfd2 = Socket(AF_INET,SOCK_STREAM,0);

server.sin_family = AF_INET;
server.sin_addr.s_addr = htonl(INADDR_ANY);
server.sin_port = htons(atoi(argv[1]));

setsockopt(sockfd2,SOL_SOCKET,SO_REUSEPORT,&(int){ 1 },sizeof(int));

Bind(sockfd2,(struct sockaddr *)&server,sizeof(server));
Listen(sockfd2,1000);


fd_set readSet, writeSet;
int fd_disponibili, fd_connectedClient[FD_SETSIZE]={0}, i;
int maxfd = sockfd2;

FD_ZERO(&readSet);
FD_ZERO(&writeSet);



while(1) {

FD_SET(sockfd2,&readSet);   //SETTO IL READSET SUL FD DELLA SOCKET IN ASCOLTO
FD_SET(STDIN_FILENO,&readSet); //Mi metto in ascolto di STDIN se voglio diventare client

//Opero su tutti i client connessi controllando l'apposito array
for(i=0; i<=maxfd; i++){
    if(fd_connectedClient[i]){
        FD_SET(i,&writeSet);  //SETTO IL WRITESET PER I CLIENT CONNESSI E A CUI POSSO "SCRIVERE"
    }
}

printf("maxfd attuale=%d\n", maxfd);
fd_disponibili = select(maxfd+1, &readSet, &writeSet, NULL, NULL);
printf("FD Disponibili=%d\n", fd_disponibili);
sleep(3);


//SE HO UN NUOVO CLIENT CONNESSO (PARTE SERVER)
if(FD_ISSET(sockfd2,&readSet)){

printf("Nuovo peer connesso\n");
accfd = Accept(sockfd2,NULL,NULL);
fd_connectedClient[accfd] = 1;//LO METTO NELLA LISTA DEI CLIENT CONNESSI
if(maxfd < accfd)
   maxfd = accfd;   //RICONTROLLO IL MAX
fd_disponibili--;

}

i = 0;


//SE PREMO INVIO DIVENTO CLIENT
if(FD_ISSET(STDIN_FILENO,&readSet)){

char readBuf[1024];
read(STDIN_FILENO,readBuf,10);  //LEGGO LO STDIN PERCHE' IL SEMPLICE FLUSH DEL BUFFER NON VA PURTROPPO...
fd_disponibili--;
printf("Sono client\n");
printf("A quale peer vuoi connetterti?\n");
read(0,readBuf,sizeof(readBuf));

int connfd = Socket(AF_INET,SOCK_STREAM,0);
client2.sin_family = AF_INET;
client2.sin_port = htons(atoi(readBuf));

if(inet_pton(AF_INET,"127.0.0.1",&client2.sin_addr) <= 0) {
  perror("Inet error");
  exit(0);
}


Connect(connfd,(struct sockaddr*)&client2,sizeof(client2));

printf("Connesso ad un altro peer\n");

char buffer[1024];
printf("Scrivere il nome della funzione del peer sulla porta %s che si vuole richiamare e scriverne i parametri distanziati da uno spazio\n", readBuf);
read(0,buffer,sizeof(buffer));
write(connfd,buffer,sizeof(buffer));
read(connfd,buffer,sizeof(buffer));
printf("%s\n",buffer);

shutdown(connfd,2);
close(connfd);  //CHIUDO IL CANALE DI COMUNICAZIONE
fflush(stdin);  //PULISCO IL BUFFER
printf("fine client\n");

}



while(fd_disponibili > 0 && i < FD_SETSIZE){

i++;

if(fd_connectedClient[i] == 0) {   //SE E' ZERO, VAI AL PROSSIMO CICLO SENZA PROSEGUIRE
    continue;
}

if(FD_ISSET(i,&writeSet)){
printf("Servo il client connesso sulla fd %d\n...\n", i);
fd_disponibili--;

char buffer[1024];
char *token, *nome_funzione, *parametro1, *parametro2;
read(accfd,buffer,sizeof(buffer));
token = strtok(buffer," ");
strcpy(token,nome_funzione);
while(token != NULL) {
strcpy(parametro1,token);
strcpy(parametro2,token);
token = strtok(NULL," ");
}

/*if(strcmp(nome_funzione,"moltiplicazione") == 0) {


}

if(strcmp(nome_funzione,"divisione") == 0) {



}

if(strcmp(nome_funzione,"frase") == 0) {



}*/

close(i);
fd_connectedClient[i] = 0;
printf("Client connesso sulla fd %d servito.\n", i);

if(maxfd == i){ //Se ho raggiunto (e servito) il maxfd devo trovare il nuovo maxfd procedendo a ritroso
    while(fd_connectedClient[--i] == 0){
    maxfd = i;
     break;
   }
}

}

}

}





return 0;
}
