#include "client.h" 

int fd;
ssize_t n;
socklen_t addrlen;

struct sockaddr_in addr;
char buffer[128];

void reg(char* IP_ADDRESS, char* PORT, char* UID, char* pass, struct addrinfo *res){
    char message[19];
    sprintf(message,"%s %s %s\n","REG",UID,pass);

    n = sendto(fd, message, strlen(message)+1, 0, res -> ai_addr, res -> ai_addrlen); // Afinal morre aqui
	if (n == -1){
        puts("Failed sending!");
        exit(EXIT_FAILURE);
    }
		
    printf("1\n");
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, strlen(buffer), 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        exit(EXIT_FAILURE);
    }
    
    if (strcmp("RRG OK\n",buffer) != 0)
        puts("There was an error in the registration. Please try again.");
	
    printf("User successfully registered");
}
