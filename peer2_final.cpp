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
#include <fstream>

using namespace std;


#define BUFF_SIZE 512

//Structure to pass multiple arguments in a thread;as thread allows only one argument to be passed...
struct arg_struct{
    int arg1;
    int arg2;
};



//The thread which will do the work of peer ka server after accepting the connections from peer ka client...
void* handle_accept(void * p_client_newsockfd){

int newsockfd=*(int *)p_client_newsockfd;

char buffer[BUFF_SIZE];
memset(buffer,'\0',BUFF_SIZE);


recv(newsockfd,buffer,sizeof(buffer),0);
printf("The file name is: %s\n",buffer);//This is the filename received from the requesting peer...

int chunkno;
int chunk_size;
recv(newsockfd,&chunkno,sizeof(int),0);//This is the chunk number received from the requesting peer...
cout<<"The chunk number is " <<chunkno<<endl;

//As all the file sending waala peers have the file; they open the file and see its' size...
FILE *fd1=fopen("test.txt","r");
fseek(fd1,0,SEEK_END);
int size_of_original_file=ftell(fd1);//The variable stores the size of the original file...
rewind(fd1);
fclose(fd1);
cout<<"The size of the original_file is "<<size_of_original_file<<endl;
chunk_size= size_of_original_file/3;//Presently dividing it by three to get the chunk size as trying to download from three peers simultaneously...
cout<<"The size of the chunksize calculated is "<<chunk_size<<endl;
send(newsockfd,&chunk_size,sizeof(chunk_size),0);//Sending the chunk size to the reciving peer...
int file_start=(chunkno-1)*chunk_size;
cout<<"The file_start calculated is "<<file_start<<endl;

FILE *fd=fopen("test.txt","r");
fseek(fd,file_start,SEEK_SET);

char Buffer2[sizeof(char)];
int temp=chunk_size;

int n;


while((n=fread(Buffer2,sizeof(char),sizeof(char),fd))>0 && temp>0){
   cout<<Buffer2<<endl;
    send(newsockfd,Buffer2,n,0);
   memset(Buffer2,'\0',sizeof(char));
    temp=temp-n;
}
rewind(fd);


close(newsockfd);
fclose(fd);
pthread_exit(0);
}


//This is the server part of the peer..Running in a separate thread...
void *peer_server(void * p_server_port_no){
    int server_port_no=*(int *)p_server_port_no;
    int server_sock_fd;
    if((server_sock_fd=socket(AF_INET,SOCK_STREAM,0)) < 0 )
    {
        //If error happens while creating the socket
        perror("Error while using socket operation for server: \n");
        exit(1);
    }
    struct sockaddr_in server_address1;//Server address structure
    struct sockaddr_in client_address;//Address Structure defined for the server to bind to a specified machine
    
    bzero((char *)&server_address1,sizeof(server_address1));
    server_address1.sin_family=AF_INET;//TCP protocol family
    server_address1.sin_port=htons(server_port_no);//Get the port number provided in the command line argument and store it in the structure
    server_address1.sin_addr.s_addr=INADDR_ANY;
    //bind the socket to our specified IP and Port


   //Binding the peer ka server to this particular socket...
    int bind_error=bind(server_sock_fd,(struct sockaddr *)&server_address1,sizeof(server_address1));
    if(bind_error<0)
        {
            perror("Bind");
            exit(1);
        }
    //listening to connection requests from clients...The 5 below signifies the maximum number of connections the server can satisfy simultaneously
    int listen_error=listen(server_sock_fd,5);
    if(listen_error<0)
    {
        perror("listen");
        exit(1);
    }
socklen_t clilen;
clilen=sizeof(client_address);
//Peer ka Server in a while Loop...
while(1){
printf("Waiting for connections\n");
int new_server_sockfd=accept(server_sock_fd,(struct sockaddr *) &client_address,&clilen);
printf("Welcome to file transfer\n");
if(new_server_sockfd < 0){
    perror("Accept");
    exit(1);

}
//Multithreading in peer ka server to handle multiple requests from other peer client part simultaneously...
pthread_t thread2;
pthread_create(&thread2,NULL,handle_accept,&new_server_sockfd);
pthread_join((struct _opaque_pthread_t *)thread2, NULL);

}//while

close(server_sock_fd);
pthread_exit(0);


}


void * handling_client(void *arguments){
    struct arg_struct *args=(struct arg_struct *)arguments;
    int portno=args->arg1;
    int chunkno=args->arg2;

    cout<<"The portno is "<<portno<<endl;
    cout<<"The chunkno is "<<chunkno<<endl;

    
    //Creating a socket to connect to the server
    int sockfd;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket");
        exit(-1);

    }
    struct sockaddr_in server_address1;
    bzero((char *) &server_address1,sizeof(server_address1));
    server_address1.sin_family=AF_INET;
    //bcopy((char *)server->h_addr,(char *)&server_address.sin_addr.s_addr,server->h_length);
    server_address1.sin_addr.s_addr=INADDR_ANY;
    server_address1.sin_port=htons(portno);
    int connect_status=connect(sockfd,(struct sockaddr *) &server_address1,sizeof(server_address1));
    if(connect_status<0)
        {
            perror("connect");
            exit(-1);
        }

//Sending the file name to the peers which I want...
char buffer[BUFF_SIZE]="test.txt";
send(sockfd,buffer,sizeof(buffer),0);
memset(buffer,'\0',sizeof(buffer));

//Sending the chunk number I want from the other peers
send(sockfd,&chunkno,sizeof(int),0);





char Buffer[BUFF_SIZE];
memset(Buffer,'\0',sizeof(Buffer));

int chunk_size;
recv(sockfd,&chunk_size,sizeof(chunk_size),0);

cout<<"The chunk_size received from the peer is :"<<chunk_size<<endl;


int file_start=(chunkno-1)*chunk_size;

cout<<"The file starting position is calculated to be :"<<file_start;

int n;

int temp=chunk_size;
FILE *fd=fopen("tesgggt1.txt","r+");
fseek(fd,file_start,SEEK_SET);

char buffer2[sizeof(char)];
memset(buffer2,'\0',sizeof(char));
while((n=recv(sockfd,buffer2,sizeof(char),0))>0 && temp > 0){
cout<<buffer2<<endl;
fwrite(buffer2,sizeof(char),n,fd);
memset(buffer2,'\0',sizeof(char));
temp=temp-n;
} 
//rewind(fd);

close(sockfd);
fclose(fd);

return NULL;


}





int main(int argc,char **argv) {
    
    if(argc <3)//check whether the port numbers are provided
    {
        fprintf(stderr,"please provide the port \n");
        exit(-1);
    }

    int server_port_no=atoi(argv[2]);
    pthread_t thread1;
    pthread_create(&thread1,NULL,peer_server,(void *)&server_port_no);
    
    
    //Creating a socket to connect to the server
    int sockfd;
    if((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("socket");
        exit(-1);

    }
    struct hostent *server;
    int portno;
    struct arg_struct args1;
    struct arg_struct args2;
    struct arg_struct args3;


FILE *fd=fopen("test.txt","r");
fseek(fd,0,SEEK_END);
int file_size=ftell(fd);
rewind(fd);
fclose(fd);

ofstream output;
output.open("tesgggt1.txt");

while(file_size>0){
    output<<'\0';
    file_size--;
    }
output.close();  
    
    pthread_t client_thread1;
    //int a,b,c,d,e,f;
    cout<<"Enter the portno of 1st peer"<<endl;
    cin>>args1.arg1;
    
    cout<<"Enter the file chunk you want from 1st peer"<<endl;
    cin>>args1.arg2;
    
    pthread_create(&client_thread1,NULL,handling_client,(void*)&args1);

    pthread_t client_thread2;
    cout<<"Enter the portno of 2nd peer"<<endl;
    cin>>args2.arg1;
    
    cout<<"Enter the file chunk you want from 2nd peer"<<endl;
    cin>>args2.arg2;
    
    pthread_create(&client_thread2,NULL,handling_client,(void*)&args2);

     pthread_t client_thread3;
    cout<<"Enter the portno of 3rd peer"<<endl;
    cin>>args3.arg1;
   
    cout<<"Enter the file chunk you want from 3rd peer"<<endl;
    cin>>args3.arg2;
    
    pthread_create(&client_thread3,NULL,handling_client,(void*)&args3);


    pthread_join((struct _opaque_pthread_t *)client_thread1, NULL);
    pthread_join((struct _opaque_pthread_t *)client_thread2, NULL);
    pthread_join((struct _opaque_pthread_t *)client_thread3, NULL);


    pthread_exit(NULL);


    
    /*server=gethostbyname(argv[1]);
    if(server==NULL){
        fprintf(stderr,"Error, no such host");
    }

    

    //specify an address for the socket to connect to
    struct sockaddr_in server_address;
    bzero((char *) &server_address,sizeof(server_address));
    server_address.sin_family=AF_INET;
    
    bcopy((char *)server->h_addr,(char *)&server_address.sin_addr.s_addr,server->h_length);
    server_address.sin_port=htons(portno);//To get the port number provided in the command line argument
    //server_address.sin_addr.s_addr=INADDR_ANY;//To get the IP address given in the command line argument and store it in the structure
    int connect_status=connect(sockfd,(struct sockaddr *) &server_address,sizeof(server_address));
    if(connect_status<0)
        {
            perror("connect");
            exit(-1);
        }
printf("%s", "Successfully connected to server\n");*/

/*char buffer[BUFF_SIZE]="SWAPNIL.jpg";
send(sockfd,buffer,sizeof(buffer),0);
memset(buffer,'\0',sizeof(buffer));

int n;
FILE *fd=fopen("test1.jpg","wb");
char Buffer[BUFF_SIZE];
int file_size;
recv(sockfd,&file_size,sizeof(file_size),0);

while((n=recv(sockfd,Buffer,BUFF_SIZE,0))>0 && file_size > 0){
//printf("%s\n",Buffer);
fwrite(Buffer,sizeof(char),n,fd);
memset(Buffer,'\0',BUFF_SIZE);
file_size=file_size-n;
} 

pthread_exit(NULL);

fclose(fd);
close(sockfd);
return 0;*/

}