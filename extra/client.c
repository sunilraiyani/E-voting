#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n,i;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
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
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
 //   printf("Please enter the message: ");
  //  bzero(buffer,256);
 //   fgets(buffer,255,stdin);
    
    unsigned int xx=1;
    memset(buffer,0,sizeof(buffer));
    buffer[1]='m';
    buffer[16]='n';
    
   // buffer[0]=1;
    
    n = write(sockfd,buffer,81);
   if (n < 0)
        error("ERROR writing to socket");
   //x bzero(buffer,256);
    n = read(sockfd,buffer,1000);
    if (n < 0) 
         error("ERROR readxing from socket");
  //  printf("%s\n",buffer);
    //printf("%d %d\n",buffer[0],buffer[16]);
    
    
    printf("%d %d\n",(int)buffer[0]);
    for(i=0;i<9;i++)
    {
        printf("mm-->%d\n",buffer[i]);
    }
    fwrite(buffer+1,1,n-1,stdout);
    if(buffer[0]==0)
        printf("bumchoot manav !");
    FILE* tmp= fopen("temp","wb");
    fwrite(buffer+1,1,16,tmp);
    
    
    return 0;
}
