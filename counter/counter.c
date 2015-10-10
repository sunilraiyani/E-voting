#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "RSA.c"
 
int main(int argc,char **argv)
	{
	int listenfd = 0,connfd = 0;
	struct sockaddr_in serv_addr;
	char sendBuff[1025];  
	char readBuff[1024];
	int n;
	unsigned int SN;
	FILE *vote;
	FILE *count;
	FILE *cand;
	char filename[1024];
	char filename1[1024];
	char filename2[1024];
	char filename3[1024];
	char filename4[1024];
	unsigned int voted;
	unsigned long long RN;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serv_addr.sin_port = htons(atoi(argv[1]));
	
	bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
	
	if(listen(listenfd, 1024) == -1 )
		{
		printf("Failed to listen\n");
		return -1;
		}


	DIR *res=opendir("result");
	struct dirent *rec;
	
	while((rec=readdir(res)))
		{
		if(rec->d_name[0] =='.' )
			{
			continue;
			}	

		sprintf(filename4,"result/%s",rec->d_name);
		remove(filename4);
		}

	res=opendir("result");

	printf("Counter is up and running!\n");
	connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); 
	while(1)
		{
		n=read(connfd,readBuff,44);
		if(n==0)
			{
			printf("Counting Done\n");
			break;
			}
		else if(n!=44)
			{
			printf("Currupted Vote!\n");
			sendBuff[0]=1;
			write(connfd, sendBuff,1);
			continue;	
			}
		memcpy(&SN,readBuff+40,4);
		sprintf(filename,"votes/%08X_vote",SN);
		printf("opening file\n");

		vote=fopen(filename,"wb");
		fwrite(readBuff,44,1,vote);
		fclose(vote);

		printf("file closed\n");
		fflush(stdout);
		if(verify(filename,"00000000_pub")==false)
			{
			printf("Voter Forged!\n");
			remove(filename);
			sendBuff[0]=2;
			write(connfd, sendBuff,1);
			continue;
			}

		sendBuff[0]=0;
		vote=fopen("temp","wb");
		fwrite(&SN,4,1,vote);
		fclose(vote);
		certify("temp","pri_key","temp1");
		vote=fopen("temp1","rb");
		fread(sendBuff+1,20,1,vote);
		write(connfd,sendBuff,21);
		remove("temp");
		remove("temp1");

		printf("vote received\n");
		fflush(stdout);

		sprintf(filename1,"%08X_temp",SN);
		sprintf(filename2,"%08X_temp1",SN);

		count=fopen(filename1,"wb");
		fwrite(readBuff+16,24,1,count);
		fclose(count);

		decrypt(filename1,"pri_key",filename2);

		count = fopen(filename2,"rb");
		fread(&voted,4,1,count);
		fread(&RN,8,1,count);
		fclose(count);
		
		remove(filename1);
		remove(filename2);
		
		printf("%llX random voted for %dth candidate\n",RN,voted);
		fflush(stdout);
		sprintf(filename3,"result/%d",voted);
		cand=fopen(filename3,"ab");
		fprintf(cand,"%016llX\n",RN);
		fclose(cand);
		}
	
	while((rec=readdir(res)))
		{
		if(rec->d_name[0] =='.' )
			{
			continue;
			}	

		sprintf(filename4,"result/%s",rec->d_name);
		cand=fopen(filename4,"rb");
		fseek(cand,0,SEEK_END);
		printf("%sth candidate got %ld votes\n",rec->d_name,ftell(cand)/17);
		}
	close(connfd);
	return 0;
	}
