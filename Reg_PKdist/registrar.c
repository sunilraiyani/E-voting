#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>

bool validate(char *x)
	{
	return true;	
	}
char info[1024*1024];
int main(int argc,char **argv)
{
	int listenfd = 0,connfd = 0;
	struct sockaddr_in serv_addr;
	char readBuff[2048];
	char sendBuff[2048];
	unsigned int invalid=0xffffffff;
	int i;
	char filename[1024];
	int info_size;
	unsigned int candidate_no=1;
	FILE *cand_list=fopen("candidate_list","r");
	FILE *voter_list=fopen("voter_list","rb");
	FILE *key;

	if(voter_list)
		{
		while(fread(info,4+32+32,1,voter_list))
				{
				candidate_no++;
				}
		fclose(voter_list);
		}
	if(cand_list)
		{
		info_size=fread(info,1,sizeof(info),cand_list);
		}
	else
		{
		printf("Candidate list not found!\n");
		return 1;	
		}
	fclose(cand_list);
	voter_list=fopen("voter_list","ab");
	
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
	printf("Registrar is up and running!\n");
	while(1)
		{
		connfd = accept(listenfd, (struct sockaddr*)NULL ,NULL); // accept awaiting request 
		read(connfd,readBuff,1);
		printf("%d\n",readBuff[0]);
		if(readBuff[0]==0)
			{
			if(read(connfd,readBuff,32+32+16)!=(32+32+16))
				{
				printf("Invalid voter registration request!\n");
				sendBuff[0]=3;
				write(connfd,sendBuff,1+sprintf(sendBuff+1,"Invalid voter registration reqeust!\0"));
				}
			else
				{
				if(!validate(readBuff+16))
					{
					printf("Voter already registered!\n");
					sendBuff[0]=4;
					write(connfd,sendBuff,1+sprintf(sendBuff+1,"Voter already registered!\0"));
					}
				else
					{
					sendBuff[0]=0;
					memcpy(sendBuff+1,&candidate_no,4);
					
					fwrite(&candidate_no,4,1,voter_list);
					fwrite(readBuff,1,64,voter_list);
					fflush(voter_list);
					sprintf(filename,"keys/%08X_pub",candidate_no);
					key=fopen(filename,"wb");
					fwrite(readBuff+64,16,1,key);
					fwrite(readBuff+64,16,1,stdout);
					fclose(key);					
					write(connfd,sendBuff,5);
					printf("%08X added!\n",candidate_no);
					candidate_no++;
					}
				}
			}
		else if(readBuff[0]==1)
			{
			printf("Candidate list returned\n");
			sendBuff[0]=1;
			strcpy(sendBuff+1, info);
			write(connfd,sendBuff,info_size+1);
			}
		else
			{
			printf("Invalid request!");	
			sendBuff[0]=2;
			write(connfd,sendBuff,1+sprintf(sendBuff+1,"Invalid reqeust!\0"));
			}
		close(connfd);
		sleep(1);
		}
	fclose(voter_list);
	return 0;
}
