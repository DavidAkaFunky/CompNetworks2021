#include "client.h" 

ssize_t bytes;
socklen_t addrlen;

struct sockaddr_in addr;

int send_and_receive(int fd, struct addrinfo *res, char* message, char* buffer, int size){
    bytes = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (bytes == -1){
        puts(SEND_ERR);
        return -1;
    } 
	addrlen = sizeof(addr);
	bytes = recvfrom(fd, buffer, size, 0, (struct sockaddr*) &addr, &addrlen);
    if (bytes == -1){
        puts(RECV_ERR);
        return -1;
    }
    return bytes;
}

void reg(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"REG %s %s\n",UID,password);
    if (send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
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
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"UNR %s %s\n", UID, password);
    if (send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
    if (!strcmp("RUN OK\n", buffer)) {
        puts("User successfully unregistered");
    } else if (!strcmp("RUN NOK\n", buffer)) {
        puts("Unregister request unsuccessful");
    } else {
        puts("There was an error in the unregistration. Please try again.");
    }    
}

int login(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"%s %s %s\n","LOG",UID,password);
    if (send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return -1;
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
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"OUT %s %s\n", UID, password);
    if (send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return -1;
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
    char message[5], buffer[SIZE]; //Tamanho do buffer provavelmente vai mudar
    memset(message, 0, 5);
    memset(buffer, 0, SIZE);
    strcpy(message,"GLS\n");
    if (send_and_receive(fd, res, message, buffer, SIZE) == -1)
        return;
    char groups[3];
    memset(message, 0, 5);
    memset(groups, 0, 3);
    sscanf(buffer, "%s %s", message, groups);
    if (!(is_correct_arg_size(groups, 2) && digits_only(groups) && !strcmp("RGL", message))){
        puts(INFO_ERR);
        return;
    }
    printf("There are %s registered groups:\n", groups);
    int n = atoi(groups);
    char group_name[25];
    char mid[5];
    char* ptr = &(buffer[6]);
    for (int i = 0; i < n; ++i){
        printf("%d\n", i);
        memset(groups, 0, 3);
        memset(group_name, 0, 25);
        memset(mid, 0, 5);
        sscanf(ptr, " %s %s %s", groups, group_name, mid);
        if (!(has_correct_arg_sizes(groups, 2, mid, 4) && digits_only(groups) && strlen(group_name) <= 24 && is_alphanumerical(group_name, 1) && digits_only(mid))){
            puts(INFO_ERR);
            return;
        }
        printf("Group ID: %s\tGroup name: %s", groups, group_name);
        for(int j = (int) strlen(group_name); j < 24; ++j){
            putchar(' '); //Manter tudo organizado por colunas
        }
        printf("Group's last message: %s\n", mid);
        ptr += 9 + strlen(group_name);
    }
    if (strcmp("\n", ptr)){
        puts(buffer);
        puts(INFO_ERR);
        return;
    }
}