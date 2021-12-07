#include "client.h" 

ssize_t n;
socklen_t addrlen;

struct sockaddr_in addr;

void reg(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    memset(message, 0, 19);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"%s %s %s\n","REG",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen); // Afinal morre aqui
	if (n == -1){
        puts("Failed sending!");
        return;
    } 
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen);
    if (n == -1){
        puts("Failed receiving!");
        return;
    }
    buffer[n] = '\0';
    if (!strcmp("RRG OK\n",buffer)) {
        puts("User successfully registered");
    } else if (!strcmp("RRG DUP\n",buffer)) {
        puts("User already registered");
    } else if (!strcmp("RRG NOK\n",buffer)) {
        puts("Registration not accepted (too many users might be registered).");
    } else {
        puts("There was an error in the registration. Please try again.");
    }
}

void unreg(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    memset(message, 0, 19);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"%s %s %s\n","UNR",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1){
        puts("Failed sending!");
        return;
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        return;
    }
    buffer[n] = '\0';
    if (!strcmp("RUN OK\n", buffer)) {
        puts("User successfully unregistered");
    } else if (!strcmp("RUN NOK\n", buffer)) {
        puts("Unregister request unsuccessful");
    } else {
        puts("There was an error in the unregistration. Please try again.");
    }    
}

int login(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    memset(message, 0, 19);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"%s %s %s\n","LOG",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1){
        puts("Failed sending!");
        return -1;
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        return -1;
    }
    buffer[n] = '\0';
    if (!strcmp("RLO OK\n",buffer)) {
        puts("User successfully logged in");
        return 1;
    } else if (!strcmp("RLO NOK\n",buffer)) {
        puts("Log in unsuccessful");
        return 0;
    } else {
        puts("There was an error logging in. Please try again.");
        return -1;
    }    
}

int logout(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    char message[19], buffer[BUF_SIZE];
    memset(message, 0, 19);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"%s %s %s\n","OUT",UID,password);
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1){
        puts("Failed sending!");
        return -1;
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        return -1;
    }
    buffer[n] = '\0';
    if (!strcmp("ROU OK\n",buffer)) {
        puts("User successfully logged out");
        return 1;
    } else if (!strcmp("ROU NOK\n",buffer)) {
        puts("Log out unsuccessful");
        return 0;
    } else {
        puts("There was an error logging out. Please try again.");
        return -1;
    }    
}

void groups(char* IP_ADDRESS, struct addrinfo *res, int fd){
    char message[4], buffer[BUF_SIZE];
    memset(message, 0, 4);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"%s\n","GLS");
    n = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (n == -1){
        puts("Failed sending!");
        return;
    }
	addrlen = sizeof(addr);
	n = recvfrom(fd, buffer, BUF_SIZE, 0, (struct sockaddr*) &addr, &addrlen); 
	if (n == -1){
        puts("Failed receiving!");
        return;
    }
    buffer[n] = '\0';
    puts(buffer);
    /* scanf dos 2 primeiros args, ver se é RGL e um número (N)
    Fazer um ciclo for N vezes em que verificamos se temos um nº de tamanho 2,
    uma string alfanumérica de tamanho <= 24 e um nº de tamanho 4.
    Se ao ler a seguir não tivermos só \n, significa que temos lixo => Erro! */
}