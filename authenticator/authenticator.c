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

	conv(counter);
	sscanf(counter,"%s %d",ip_t,&port_t);
	counter_addr.sin_family = AF_INET;
	counter_addr.sin_port = htons(port_t);
	counter_addr.sin_addr.s_addr = inet_addr(ip_t);
	sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd1, (struct sockaddr *)&counter_addr, sizeof(counter_addr));

	

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
		if(!access(filename,R_OK))
			{
			printf("Already voted\n");
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
				rec=fopen(filename2,"rb");
				fread(sendBuff+1,68,1,rec);
				write(connfd,sendBuff,69);
				fclose(rec);
				remove(filename2);
				
				sprintf(filename3,"%08X_auth_count",SN);
				sprintf(filename4,"%08X_auth_count1",SN);
				vote_=fopen(filename3,"wb");
			
				memcpy(ballot+24,&SN,4);
				fwrite(ballot,28,1,vote_);
				fclose(vote_);
				
				certify(filename3,"pri_key",filename4);
				vote_=fopen(filename4,"rb");
				fread(sendBuff,28+16,1,vote_);
				fclose(vote_);
				remove(filename3);
				remove(filename4);
				

                printf("time to send to counter %d\n",sockfd1);
                fflush(stdout);
                    
				write(sockfd1,sendBuff,28+16);

				printf("Vote sent to counter \n");
				fflush(stdout);
											
				read(sockfd1,readBuff,1);
				if(readBuff[0]==0)
					{
					read(sockfd1,readBuff,20);
					sprintf(filename5,"receipts/%08X_count_rec",SN);
					rec_=fopen(filename5,"wb");
					fwrite(readBuff,20,1,rec_);
					fclose(rec_);
					printf("%08X receipt received!\n",SN);
					SN++;
					}
				else
					{
					printf("Error %d occured!\n",readBuff[0]);	
					}
				}
			else
				{
				printf("Forged vote!\n");
				continue;	
				}
			remove(filename1);
			}
		else
			{
			printf("error!\n");
			continue;	
			}
		sleep(1);
		}	

	return 0;
}
