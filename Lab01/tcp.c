#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define PORT "58001"

int fd, errcode,newfd;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[128];

int main() {

	fd = socket (AF_INET, SOCK_STREAM,0);
	if (fd == -1)
		exit(EXIT_FAILURE);
	memset(&hints, 0, sizeof hints);
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;

	errcode=getaddrinfo(NULL,PORT,&hints,&res);
	if((errcode)!=0)
		exit(EXIT_FAILURE);

	n=bind(fd,res->ai_addr,res->ai_addrlen);
	if (n==-1)
		exit(EXIT_FAILURE);

	if (listen(fd,5)==-1)
		exit(EXIT_FAILURE);
	while(1) {
		addrlen=sizeof(addr);
		if((newfd=accept(fd,(struct sockaddr*)&addr,&addrlen)) ==-1)
			exit(EXIT_FAILURE);
		n=read(newfd,buffer,128);
		if (n==-1)
			exit(EXIT_FAILURE);
		write(1,"received: ",10); write (1,buffer,n);
		if(n == 10){
			char x[44];
			sprintf(x, "ok, we give up: FLAG{th15_0n3_w45_tr1ck13r}\n");
			n=write(newfd,x,44);
			if(n==-1)
				exit(EXIT_FAILURE);
		}
		
		close(newfd);
	}
	freeaddrinfo(res);
	close(fd);

}