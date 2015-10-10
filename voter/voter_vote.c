#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include "RSA.c"

void conv(char x[30])
	{
	int i;
	for(i=0;x[i]!=':';i++);
	x[i]=' ';
	return;	
	}
int main()
	{
	int sockfd = 0,n = 0;
	char sendBuff[1024];
	char readBuff[1024];
	struct sockaddr_in serv_addr;
	FILE *ip=fopen("ip.config","r");
	
	srand(time(0));
	char PKdist[30];
	char registrar[30];
	char auth[30];
	char counter[30];
	char ip_t[30];
	int port_t;
	
	char _pub[16];
	char name[33]={0};
	char email[33]={0};
	unsigned int voterID;
	unsigned int authID=0;
	unsigned int counterID=0xffffffff;
	
	char x;
	unsigned int cand_choice;
	unsigned long long RN;
	int i,status;
	fgets(registrar,30,ip);
	fgets(PKdist,30,ip);
	fgets(auth,30,ip);
	fgets(counter,30,ip);
	fclose(ip);
	
	serv_addr.sin_family = AF_INET;
	
	FILE *id=fopen("data/id","rb");
	if(id==0)
		{
		printf("ID not found!\n");
		return 1;
		}	
	fread(&voterID,4,1,id);
	fclose(id);
	
	conv(registrar);
	sscanf(registrar,"%s %d",ip_t,&port_t);
	serv_addr.sin_port = htons(port_t);
	serv_addr.sin_addr.s_addr = inet_addr(ip_t);

///// reading candidate list
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	sendBuff[0]=1;
	write(sockfd,sendBuff,1);
	read(sockfd,readBuff,1);
	if(readBuff[0]==1)
		{
		while((n=read(sockfd,readBuff,sizeof(readBuff)))==sizeof(readBuff))
				{
				fwrite(readBuff,1,n,stdout);	
				}
		fwrite(readBuff,1,n,stdout);		
		}
	else
		{
		read(sockfd,readBuff,sizeof(readBuff));
		printf("%s",readBuff+1);	
		}
	close(sockfd);

///// Ballot generation
	printf("Enter candidate number you want to vote: ");
	scanf("%u",&cand_choice);
	printf("Do you want to enter your own Random? (y/n):");
	scanf(" %c",&x);
	if(x!='y')
		{
		while(1)
			{	
			RN=randomn();
			printf("Random generated: %llX\n",RN);
			printf("Choose another random? (y/n): ");
			scanf(" %c",&x);
			x=tolower(x);
			if(x!='y')
				break;
			}
		}
	else
		{
		printf("Enter Random in hex: ");
		scanf("%llx",&RN);
		}
//// ballot generation and signing

	FILE *counter_key=fopen("FFFFFFFF_pub","wb");
	conv(PKdist);
	sscanf(PKdist,"%s %d",ip_t,&port_t);
	serv_addr.sin_port = htons(port_t);
	serv_addr.sin_addr.s_addr = inet_addr(ip_t);	
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	
	write(sockfd,&counterID,4);
	if(read(sockfd,readBuff,17)==17 && readBuff[0]==0)
		{
		fwrite(readBuff+1,16,1,counter_key);
		}
	else
		{
		printf("fatal error!\n");
		return 2;	
		}
	fclose(counter_key);
	FILE *tmp=fopen("ballot_0","wb");
	fwrite(&cand_choice,4,1,tmp);
	fwrite(&RN,8,1,tmp);
	fclose(tmp);
	encrypt1("ballot_0","FFFFFFFF_pub","ballot_1");
	
	
	tmp=fopen("ballot_1","ab");

	fwrite(&voterID,4,1,tmp);
	time_t now=time(NULL);
	fwrite(&now,8,1,tmp);
	
	fclose(tmp);
	certify("ballot_1","pri_key","ballot");
	
	close(sockfd);
///// send to authanticator and receive the receipt 
	conv(auth);
	sscanf(auth,"%s %d",ip_t,&port_t);
	serv_addr.sin_port = htons(port_t);
	serv_addr.sin_addr.s_addr = inet_addr(ip_t);	
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));	
	
	FILE *ballot=fopen("ballot","rb");
	if(fread(sendBuff,52,1,ballot)!=1)
		{
		printf("fatal error!\n ballot is currupted\n");
		return 3;
		}
	write(sockfd,sendBuff,52);
	if((n=read(sockfd,readBuff,69))!=69)
		{
		if(readBuff[0]==11)
			{
			printf("You have already voted!\n");
			return 4;	
			}
		if(readBuff[0]==1)
			{
			printf("You are caught in Forgery!\n");
			return 5;
			}
		if(readBuff[0]==22)
			{
			printf("You are not Registered!\n");
			return 6;	
			}
		printf("fatal error!\n receipt is currupted\n");
		return 10;
		}
//	printf("n=%d\n",n);
	FILE *rec=fopen("receipt","wb");
	fwrite(readBuff+1,68,1,rec);
	fclose(rec);
	printf("Receipt received!\n");
	return 0;
	}
