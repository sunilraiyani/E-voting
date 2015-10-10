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
int main(int argc,char **argv)
{
	int listenfd = 0,connfd = 0;
	int sockfd=0;
	int sockfd1=0;
	struct sockaddr_in serv_addr;
	struct sockaddr_in counter_addr;
	struct sockaddr_in PKdist_addr;
	
	char readBuff[2048];
	char sendBuff[2048];
	char ballot[1024];
	char filename[1024];
	char filename1[1024];
	char filename2[1024];
	char filename3[1024];
	char filename4[1024];
	char filename5[1024];
	
	char PKdist[30];
	char registrar[30];
	char auth[30];
	char counter[30];
	char ip_t[30];
	int port_t;

	
	FILE *ip=fopen("ip.config","r");
	FILE *vote_list=fopen("votes/votes_list","ab");
	FILE *vote,*key,*rec,*vote_;
	FILE *rec_;
	fgets(registrar,30,ip);
	fgets(PKdist,30,ip);
	fgets(auth,30,ip);
	fgets(counter,30,ip);
	fclose(ip);
	
	
	
	unsigned int SN=0;
	unsigned int voterID;

	
	conv(PKdist);
	sscanf(PKdist,"%s %d",ip_t,&port_t);
	PKdist_addr.sin_family = AF_INET;
	PKdist_addr.sin_port = htons(port_t);
	PKdist_addr.sin_addr.s_addr = inet_addr(ip_t);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *)&PKdist_addr, sizeof(PKdist_addr));
	

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff));
	serv_addr.sin_family = AF_INET;    
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(atoi(argv[1]));
	
	bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));

	if(listen(listenfd, 1024) == -1 )
		{
		printf("Failed to listen\n");
		return -1;
		}

	printf("Authenticator is up and running!\n");
	fflush(stdout);

	while(1)
		{
		connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL);
		read(connfd,readBuff,52);
		memcpy(&voterID,readBuff+16+24,4);
		printf("Voter %08X vote received!\n",voterID);
		sprintf(filename,"votes/%08X_vote",voterID);
		
		//cheking for duplicate votes
		ip=fopen(filename,"rb");

		if(ip!=0)
			{
			sendBuff[0]=11;
			printf("Already voted\n");
			fflush(stdout);
			write(connfd,sendBuff,1);
			fclose(ip);
			continue;	
			}
		
		vote=fopen(filename,"wb");
		fwrite(readBuff,52,1,vote);
		fclose(vote);
		memcpy(ballot,readBuff+16,24);
///// ask public key of voter from PKdist
		write(sockfd,&voterID,4);
		read(sockfd,readBuff,17);
		if(readBuff[0]==0)
			{
			sprintf(filename1,"%08X_pub",voterID);
			key=fopen(filename1,"wb");
			fwrite(readBuff+1,16,1,key);
			fclose(key);
			
			if(verify(filename,filename1))
				{
				sendBuff[0]=0;
				sprintf(filename2,"%08X_rec",voterID);
				certify(filename,"pri_key",filename2);
				fwrite(&voterID,4,1,vote_list);
				fflush(vote_list);
				rec=fopen(filename2,"rb");
				fread(sendBuff+1,68,1,rec);
				write(connfd,sendBuff,69);
				fclose(rec);
				remove(filename2);
				}
			else
				{
				printf("Forged vote!\n");
				sendBuff[0]=1;
				remove(filename);
				write(connfd,sendBuff,1);
				continue;	
				}
			remove(filename1);
			}
		else
			{
			printf("VoterID not found!\n");
			sendBuff[0]=22;
			write(connfd,sendBuff,1);
			remove(filename);
			continue;	
			}
		sleep(1);
		}	

	return 0;
}
