#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

int prime[1000001]={0};
int plist[1000000];
int pcnt=0;

long long big_mult(long long a,long long b,long long mod)
	{
	long double ans;
	long long c;
	a=a%mod;
	b=b%mod;
	ans=(long double)a*b;
	a=a*b;
	c=(long long)(ans/mod);
	a=a-c*mod;
	 
	a=a%mod;
	if(a<0)
		a=a+mod;
	 
	return a;
	}


long long modpow(long long base,long long exp,long long mod)
	{
	long long ans=1;
	while(exp)
		{
		if(exp%2==1)
			{
			ans=big_mult(ans,base,mod);
			}
		base=big_mult(base,base,mod);
		exp/=2;
		}
	return ans;
	}

long long gcd(long long a,long long b)
	{
	if(b==0)
		return a;
	return gcd(b,a%b);
	}
	
	
long long inv(long long m,long long b)
	{
	long long int q,a1,a2,a3,b1,b2,b3,t1,t2,t3;
	a1=1;
	a2=0;
	b1=0;
	b2=1;
	a3=m;
	b3=b;
	q=-1;
	while(b3>1)
		{
		if(b3==0)
			{
			b2=a3;
			break;
			}
		else if(b3==1)
			{
			break;
			}
		q=a3/b3;
		t1=a1-(q*b1);
		t2=a2-(q*b2);
		t3=a3-(q*b3);
		a1=b1;a2=b2;a3=b3;
		b1=t1;b2=t2;b3=t3;
		}
	b2%=m;
	b2=(b2+m)%m;
	return b2;
	}
	
	
long long randomn()
	{
	long long x=0,y;
	int i=0;
	for(i=0;i<8;i++)
		{
		y=rand()%(1<<8);
		x=x<<8;
		x|=y;
		}
	if(x<0)
		return -x;
	return x;
	}
	
	
void keygen()
	{
	srand(time(0));
	int i,j;
	FILE *pub=fopen("pub_key","wb");
	FILE *pri=fopen("pri_key","wb");
	for(i=2;i*i<=1000000;i++)
		{
		if(prime[i]!=0)
			continue;
		for(j=i*2;j<=1000000;j+=i)
			{
			prime[j]=1;
			}
		}
	for(i=100000;i<=1000000;i++)
		if(prime[i]==0)
			plist[pcnt++]=i;
	
	long long p,q;
	p=plist[randomn()%pcnt];
	q=plist[randomn()%pcnt];
	long long n=p*q;
	long long phi=(p-1)*(q-1);
	long long e=randomn()%(phi-2)+2;

	while(gcd(e,phi)!=1)
		e=rand()%(phi-2)+2;

	long long d=inv(phi,e);

	fwrite(&e,sizeof(e),1,pri);
	fwrite(&n,sizeof(n),1,pri);
	fwrite(&d,sizeof(d),1,pub);
	fwrite(&n,sizeof(n),1,pub);
	
//	printf("%llu %llu %llu %llu\n",e,d,phi,n);
//	printf("%llu %llu\n",d,n);
	fclose(pri);
	fclose(pub);
	}

void hash(char *in_file,char *out_file)
	{
	FILE *in=fopen(in_file,"rb");
	FILE *out=fopen(out_file,"wb");
	char buff[16];
	char buff1[8];
	int status;
	int i;
	memset(buff,0,16);
	memset(buff1,0,8);
	while((status=fread(buff,1,16,in))==16)
		{
		for(i=0;i<16;i++)
			{
			buff1[i/2]^=buff[i];	
			}
		memset(buff,0,16);
		}
	if(status!=0)
		{
		for(i=0;i<16;i++)
			{
			buff1[i/2]^=buff[i];	
			}	
		}
	fwrite(buff1,1,8,out);
	fclose(out);
	}

void decrypt(char *in_file,char *key_file,char *out_file)
	{
	FILE *in=fopen(in_file,"rb");
	FILE *_key=fopen(key_file,"rb");
	FILE *out=fopen(out_file,"wb");
	long long key;
	long long n;
	long long chunk;
	long long enc_chunk;
	unsigned int r;
	fread(&key,sizeof(key),1,_key);	
	fread(&n,sizeof(n),1,_key);
//	printf("%llu %llu\n",key,n);
	while(fread(&enc_chunk,8,1,in))
		{
//		printf("%llu\n",enc_chunk);
		chunk=modpow(enc_chunk,key,n);
//		printf("%llu\n",chunk);
		r=chunk;
		fwrite(&r,sizeof(r),1,out);
		}
	fclose(out);
	}

void encrypt1(char *in_file,char *key_file,char *out_file)
	{
	FILE *in=fopen(in_file,"rb");
	FILE *_key=fopen(key_file,"rb");
	FILE *out=fopen(out_file,"wb");
	long long key;
	long long n;
	long long chunk;
	long long rchunk;
	long long enc_chunk;
	unsigned char x;
	int i=0;
	fread(&key,sizeof(key),1,_key);	
	fread(&n,sizeof(n),1,_key);
//	printf("-%llu %llu\n",key,n);
	while(fread(&x,1,1,in))
		{
		if(i==0)
			{
			chunk=0;
			}
		chunk=chunk<<8;
		chunk|=x;
		if(i==3)
			{
			rchunk=(chunk&0x000000FF);
			rchunk<<=8;
			rchunk|=(chunk&0x0000FF00)>>8;
			rchunk<<=8;
			rchunk|=(chunk&0x00FF0000)>>16;
			rchunk<<=8;
			rchunk|=(chunk&0xFF000000)>>24;
			
//			printf("-%llu\n",rchunk);
			enc_chunk=modpow(rchunk,key,n);
//			printf("-%llu\n",enc_chunk);
			fwrite(&enc_chunk,sizeof(enc_chunk),1,out);
			}
		i++;
		i%=4;
		}
	if(i!=0)
		{
		enc_chunk=modpow(chunk,key,n);
		fwrite(&enc_chunk,sizeof(enc_chunk),1,out);		
		}
	fclose(out);
	}

void certify(char *in_file,char *key_file,char *out_file)
	{
	char buff[16];

	hash(in_file,"hash");
	encrypt1("hash",key_file,"ehash");
	
	FILE* out=fopen(out_file,"wb");
	FILE* thash=fopen("ehash","rb");
	FILE* in=fopen(in_file,"rb");
	
	fread(buff,1,16,thash);
	fwrite(buff,1,16,out);
	
	while(fread(buff,1,1,in))
		{
		fwrite(buff,1,1,out);	
		}
	fclose(out);
	remove("hash");
	remove("ehash");
	}	

bool verify(char *in_file,char *key_file)
	{
	FILE *temp=fopen(in_file,"rb");
	FILE *msg_temp=fopen("tmp","wb");
	FILE *hash_temp=fopen("khash","wb");
	char buff[16];
	char x;
	int i=0;
	fread(buff,1,16,temp);
	fwrite(buff,1,16,hash_temp);
	while(fread(&x,1,1,temp))
		{
		fwrite(&x,1,1,msg_temp);	
		}
	fclose(msg_temp);
	fclose(temp);
	fclose(hash_temp);
	
	decrypt("khash",key_file,"hash1");
	hash("tmp","hash2");
	FILE *h1=fopen("hash1","rb");
	FILE *h2=fopen("hash2","rb");
	bool flag=true;
	for(i=0;i<8;i++)
		{
		if(fread(buff,1,1,h1) && fread(buff+1,1,1,h2));
		else
			{
			flag=false;	
			break;
			}
		if(buff[0]!=buff[1])
			{
			flag=false;	
			break;	
			}
		}
	remove("tmp");
	remove("hash1");
	remove("khash");
	remove("hash2");
	return flag;
	}
/*
int main()
{
keygen();
//encrypt("manav.txt","pub_key","c.txt");
//decrypt("c.txt","pri_key","d.txt");
certify("manav.txt","pri_key","certi");
printf("status %d\n",verify("certi","pub_key"));
return 0;
}
*/
