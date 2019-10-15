#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <iostream>
#include <unordered_map>
using namespace std;

#define BUFF_SIZE 255

unordered_map<int,string> data_structure;//This is the data structure used to hold the pair of port,user and the file-name it has....



void* handle_accept(void * p_client_newsockfd);








int main(int argc,char ** argv) {


    //To throw error when port is not provided in the command line argument...
    if(argc<2){
        fprintf(stderr,"please provide the port\n");
        exit(1);
    }
    

    int sockfd;//sockfd is socket descriptor for server
    
    //Creating the server socket and storing the socket descriptor in sockfd
    int portno;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0 )
    {
        //If error happens while creating the socket
        perror("Error while using socket operation for server: \n");
        exit(1);
    }

    //defining the address structure
    struct sockaddr_in server_address;//Server address structure
    struct sockaddr_in client_address;//Address Structure defined for the server to bind to a specified machine
    
    bzero((char *)&server_address,sizeof(server_address));

    portno=atoi(argv[1]);//Get the port number
    
    server_address.sin_family=AF_INET;//TCP protocol family
    server_address.sin_port=htons(portno);//Get the port number provided in the command line argument and store it in the structure
    server_address.sin_addr.s_addr=INADDR_ANY;//INADDR_ANY for binding the server to any  netwrok interface for given port no


    //bind the socket to our specified IP and Port
    int bind_error=bind(sockfd,(struct sockaddr *) &server_address,sizeof(server_address));
    if(bind_error<0)
        {
            perror("Bind");
            exit(1);
        }
    //listening to connection requests from clients...The 5 below signifies the maximum number of connections the server can satisfy simultaneously
    int listen_error=listen(sockfd,5);
    if(listen_error<0)
    {
        perror("listen");
        exit(1);
    }
socklen_t clilen;
clilen=sizeof(client_address);

while(1){
    printf("Waiting for connections\n");


int newsockfd=accept(sockfd,(struct sockaddr *) &client_address,&clilen);

if(newsockfd < 0){
    perror("Accept");
    exit(1);

}
//handle_accept(newsockfd);

//Multithreading happening
pthread_t t;
//int *pclient=(int*)malloc(sizeof(int));
//*pclient=newsockfd;
pthread_create(&t,NULL,handle_accept,&newsockfd);
pthread_join((struct _opaque_pthread_t *)t, NULL);

}//while

close(sockfd);

return 0;
}



//Messaging Service



void* handle_accept(void * p_client_newsockfd){
int* newsockfd1=(int *) p_client_newsockfd;
int newsockfd=*newsockfd1;
char buffer[255];
memset(buffer,'\0',255);


recv(newsockfd,buffer,sizeof(buffer),0);
cout<<"The command is "<<buffer<<endl;


int port;
recv(newsockfd,&port,sizeof(int),0);
cout<<"The port is "<<port<<endl;

char buffer2[255];
recv(newsockfd,buffer2,sizeof(buffer2),0);
cout<<"The file_name that the client has is"<<buffer2<<endl;
if(buffer== "create user"){
    data_structure[port]=buffer2;
}


close(newsockfd);

pthread_exit(0);
}