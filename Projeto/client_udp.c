#include "client.h" 

void reg(char* IP_ADDRESS, char* PORT,char* UID, char* pass){
    char* message[19];
    sprintf(message[0],"%s %s %s\n","REG",UID,pass);

    n = sendto(fd, message, 18, 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1)
		exit(EXIT_FAILURE);
    printf("1\n");
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, 128, 0, (struct sockaddr*) &addr, &addrlen); //morre aqui
	if (n == -1)
		exit(EXIT_FAILURE);
    
    if (strcmp("RRG OK\n",buffer) != 0)
        puts("There was an error in the registration. Please try again.");
	
    printf("User successfully registered");
}
