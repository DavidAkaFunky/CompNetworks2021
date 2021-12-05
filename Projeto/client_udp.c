#include "client.h" 

ssize_t n;
socklen_t addrlen;

struct sockaddr_in addr;
char buffer[128];

void reg(char* IP_ADDRESS, char* PORT, char* UID, char* pass, struct addrinfo *res, int fd){
    char message[19];
    sprintf(message,"%s %s %s\n","REG",UID,pass);
    printf("%d\n", res -> ai_addrlen);
    n = sendto(fd, message, strlen(message)+1, 0, res -> ai_addr, res -> ai_addrlen); // Afinal morre aqui
	if (n == -1){
        puts("Failed sending!");
        exit(EXIT_FAILURE);
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, strlen(buffer), 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        exit(EXIT_FAILURE);
    }
    if (strcmp("RRG OK\n",buffer) != 0){
        puts("There was an error in the registration. Please try again.");
        exit(EXIT_FAILURE);
    }      
    puts("User successfully registered");
}
