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

void swap(int *a,int *b)
	{
	int t=*a;
	*a=*b;
	*b=t;	
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
	
	int i,j;
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


	printf("Authenticator is sending votes to counter!\n");
	fflush(stdout);

	FILE *vote_list=fopen("votes/votes_list","rb");
	FILE *map=fopen("vote_permute","wb");
	int vote_count=0;

	while(fread(&voterID,4,1,vote_list))
		vote_count++;
	
	int *shuffler=(int *)malloc(sizeof(int)*vote_count);
	for(i=0;i<vote_count;i++)
		shuffler[i]=i;
	
	srand(time(0));
	
	for(i=0;i<vote_count;i++)
		{
		j=rand()%(i+1);
		if(i!=j)
				{
				swap(&shuffler[i],&shuffler[j]);	
				}
		}
	for(i=0;i<vote_count;i++)
		{
		fprintf(map,"%d\n",shuffler[i]);
		}

//	fseek(vote_list,offset,SEEK_SET);
	for(SN=0;SN<vote_count;)
		{
		fseek(vote_list,sizeof(int)*shuffler[SN],SEEK_SET);
		fread(&voterID,4,1,vote_list);
		printf("%08X vote passed\n",voterID);
		sprintf(filename,"votes/%08X_vote",voterID);
		vote=fopen(filename,"rb");
		fread(readBuff,52,1,vote);
		fclose(vote);
		memcpy(ballot,readBuff+16,24);

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

	return 0;
}
