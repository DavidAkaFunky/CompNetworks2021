#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#define PORT "58001"

int fd, errcode;
ssize_t n;
socklen_t addrlen;
struct addrinfo hints, *res;
struct sockaddr_in addr;
char buffer[128];

int main(){
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
		exit(EXIT_FAILURE);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	errcode = getaddrinfo("guadiana.tecnico.ulisboa.pt", PORT, &hints, &res);
	if (errcode != 0)
		exit(EXIT_FAILURE);

	n = sendto(fd, "123456\n", 7, 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1)
		exit(EXIT_FAILURE);

	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen);
	if (n == -1)
		exit(EXIT_FAILURE);

	write(1, "echo: ", 6);
	write(1, buffer, n);
	sprintf(buffer, "\0");

	freeaddrinfo(res);
	close(fd);
	exit(EXIT_SUCCESS);
}