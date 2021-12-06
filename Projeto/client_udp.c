#include "client.h" 

ssize_t n;
socklen_t addrlen;

struct sockaddr_in addr;

void reg(char* IP_ADDRESS, char* PORT, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    sprintf(message,"%s %s %s\n","REG",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen); // Afinal morre aqui
	if (n == -1){
        puts("Failed sending!");
        exit(EXIT_FAILURE); //Substituir por return?
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
    if (n == -1){
        puts("Failed receiving!");
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    if (!strcmp("RRG OK\n",buffer)) {
        puts("User successfully registered");
    } else if (!strcmp("RRG DUP\n",buffer)) {
        puts("User already registered");
    } else if (!strcmp("RRG NOK\n",buffer)) {
        puts("Registration not accepted (too many users might be registered).");
    } else {
        puts(buffer);
        puts("There was an error in the registration. Please try again.");
        exit(EXIT_FAILURE);
    }
}

void unreg(char* IP_ADDRESS, char* PORT, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    sprintf(message,"%s %s %s\n","UNR",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1){
        puts("Failed sending!");
        exit(EXIT_FAILURE);
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    if (!strcmp("RUN OK\n", buffer)) {
        puts("User successfully unregistered");
    } else if (!strcmp("RUN NOK\n", buffer)) {
        puts("Unregister request unsuccessful");
    } else {
        puts(buffer);
        puts("There was an error in the unregistration. Please try again.");
        exit(EXIT_FAILURE);
    }    
}

int login(char* IP_ADDRESS, char* PORT, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    sprintf(message,"%s %s %s\n","LOG",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1){
        puts("Failed sending!");
        exit(EXIT_FAILURE);
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    if (!strcmp("RLO OK\n",buffer)) {
        puts("User successfully logged in");
        return 1;
    } else if (!strcmp("RLO NOK\n",buffer)) {
        puts("Log in unsuccessful");
        return 0;
    } else {
        puts(buffer);
        puts("There was an error logging in. Please try again.");
        exit(EXIT_FAILURE); //ou return -1
    }    
}

int logout(char* IP_ADDRESS, char* PORT, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    sprintf(message,"%s %s %s\n","OUT",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1){
        puts("Failed sending!");
        exit(EXIT_FAILURE);
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0';
    if (!strcmp("ROU OK\n",buffer)) {
        puts("User successfully logged out");
        return 1;
    } else if (!strcmp("ROU NOK\n",buffer)) {
        puts("Log out unsuccessful");
        return 0;
    } else {
        puts(buffer);
        puts("There was an error logging out. Please try again.");
        exit(EXIT_FAILURE); //ou return -1
    }    
}