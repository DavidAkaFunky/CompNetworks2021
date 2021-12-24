#include "client.h" 

ssize_t bytes;
socklen_t addrlen;
struct sockaddr_in addr;

int udp_send_and_receive(int fd, struct addrinfo *res, char* message, char* buffer, int size){
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
    if (!(digits_only(UID,"UID") && has_correct_arg_sizes(UID, 5, password, 8) && is_alphanumerical(password, 0)))
        return;
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"REG %s %s\n",UID,password);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
    if (!strcmp("ERR\n", buffer))
        puts(GEN_ERR);
    else if (!strcmp("RRG OK\n",buffer))
        puts(REG_USER_SUC);
    else if (!strcmp("RRG DUP\n",buffer))
        puts(REG_USER_DUP);
    else if (!strcmp("RRG NOK\n",buffer))
        puts(REG_USER_FAIL);
    else
        puts(INFO_ERR);
}

void unreg(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    if (!(digits_only(UID,"UID") && has_correct_arg_sizes(UID, 5, password, 8) && is_alphanumerical(password, 0)))
        return;
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"UNR %s %s\n", UID, password);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
    if (!strcmp("ERR\n", buffer))
        puts(GEN_ERR);
    if (!strcmp("RUN OK\n", buffer)) {
        puts(UNR_USER_SUC);
    } else if (!strcmp("RUN NOK\n", buffer)) {
        puts(UNR_USER_FAIL);
    } else {
        puts(INFO_ERR);
    }    
}

int login(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    if (!(digits_only(UID,"UID") && has_correct_arg_sizes(UID, 5, password, 8) && is_alphanumerical(password, 0)))
        return -1;
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"LOG %s %s\n",UID,password);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return -1;
    if (!strcmp("ERR\n", buffer)){
        puts(GEN_ERR);
        return -1;
    }
    if (!strcmp("RLO OK\n",buffer)) {
        puts(LOGIN_SUC);
        return 1;
    }
    if (!strcmp("RLO NOK\n",buffer)) {
        puts(LOGIN_FAIL);
        return 0;
    }
    puts(INFO_ERR);
    return -1;
}

int logout(char* IP_ADDRESS, char* UID, char* password, struct addrinfo *res, int fd){
    char message[20], buffer[BUF_SIZE];
    memset(message, 0, 20);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"OUT %s %s\n", UID, password);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return -1;
    if (!strcmp("ERR\n", buffer)){
        puts(GEN_ERR);
        return -1;
    }
    if (!strcmp("ROU OK\n",buffer)) {
        puts(LOGOUT_SUC);
        return 1;
    }
    if (!strcmp("ROU NOK\n",buffer)) {
        puts(LOGOUT_FAIL);
        return 0;
    }
    puts(INFO_ERR);
    return -1;
  
}

void show_groups(char* ptr, char* groups){
    int n = atoi(groups), format;
    char group_name[25];
    char mid[5];
    for (int i = 0; i < n; ++i){
        memset(groups, 0, 3);
        memset(group_name, 0, 25);
        memset(mid, 0, 5);
        format = sscanf(ptr, " %s %s %s", groups, group_name, mid);
        if (!(format == 3 && has_correct_arg_sizes(groups, 2, mid, 4) && digits_only(groups, "GID") && strlen(group_name) <= 24 && is_alphanumerical(group_name, 1) && digits_only(mid, "MID"))){
            puts(INFO_ERR);
            return;
        }
        printf("Group ID: %s\tGroup name: %s", groups, group_name);
        for(int j = (int) strlen(group_name); j < 26; ++j){
            putchar(' '); //Manter tudo organizado por colunas
        }
        printf("Group's last message: %s\n", mid);
        ptr += 9 + strlen(group_name);
    }
    if (strcmp(ptr, "\n"))
        puts(INFO_ERR);
}

void groups(char* IP_ADDRESS, struct addrinfo *res, int fd){
    char buffer[GROUPS];
    memset(buffer, 0, GROUPS);
    if (udp_send_and_receive(fd, res, "GLS\n", buffer, GROUPS) == -1)
        return;
    char response[4], groups[3];
    memset(response, 0, 4);
    memset(groups, 0, 3);
    if (!strcmp("ERR\n", buffer)){
        puts(GEN_ERR);
        return;
    }
    int format = sscanf(buffer, "%s %s", response, groups);
    if (!(format == 2 && !strcmp("RGL", response) && (strlen(groups) == 1 || strlen(groups) == 2) && digits_only(groups, "number of groups"))){
        puts(INFO_ERR);
        return;
    }
    printf("There are %s registered %s:\n", groups, strcmp(groups, "1") ? "groups" : "group");
    show_groups(&(buffer[4+strlen(groups)]), groups);
}

void subscribe(char* IP_ADDRESS, char* UID, char* GID, char* GName, struct addrinfo *res, int fd){
    if (strlen(GID) == 1)
        sprintf(GID, "0%c", GID[0]);
    if (!(is_correct_arg_size(GID, 2) && digits_only(GID,"GID") && strlen(GName) <= 24 && is_alphanumerical(GName, 1)))
        return;
    char message[38], buffer[BUF_SIZE];
    memset(message, 0, 38);
    memset(buffer, 0, BUF_SIZE);
    sprintf(message,"GSR %s %s %s\n", UID, GID, GName);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
    if (!strcmp("ERR\n", buffer))
        puts(GEN_ERR);
    else if (!strcmp("RGS OK\n",buffer)) 
        printf("You have successfully subscribed to group %s!\n", GID);
    else if (!strcmp("RGS E_GRP\n",buffer))
        puts(GRP_FAIL);
    else if (!strcmp("RGS E_GNAME\n",buffer))
        puts(REG_GRP_INV);
    else if (!strcmp("RGS E_FULL\n",buffer))
        puts(REG_GRP_FULL);
    else if (!strcmp("RGS NOK\n",buffer)) 
        puts(REG_GRP_ERR1);
    else {
        char cmd1[4], cmd2[4], new_GID[3], extra[SIZE];
        memset(cmd1, 0, 4);
        memset(cmd2, 0, 4);
        memset(new_GID, 0, 3);
        memset(extra, 0, SIZE);
        int format = sscanf(buffer, "%s %s %s %s", cmd1, cmd2, new_GID, extra);
        if (!(format == 3 && !strcmp(cmd1, "RGS") && !strcmp(cmd1, "NEW") && is_correct_arg_size(new_GID, 2) && !strcmp(extra, ""))){
            strcpy(GID, new_GID);
            printf("New group created with GID %s!\n", new_GID);
        }
        else
            puts(INFO_ERR);
    }
}

void unsubscribe(char* IP_ADDRESS, char* UID, char* GID, struct addrinfo *res, int fd) {
    char message[13], buffer[BUF_SIZE];
    sprintf(message,"GUR %s %s\n", UID, GID);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
    if (!strcmp("ERR\n", buffer))
        puts(GEN_ERR);
    else if (!strcmp("RGU OK\n",buffer)) 
        printf("You have successfully unsubscribed from group %s.\n", GID);
    else if (!strcmp("RGU E_USR\n",buffer))
        puts(UNR_GRP_FAIL_USR);
    else if (!strcmp("RGU E_GRP\n",buffer))
        puts(GRP_FAIL);
    else if (!strcmp("RGU NOK\n",buffer)) 
        puts(UNR_GRP_ERR1);
    else
        puts(INFO_ERR);
}

void my_groups(char* IP_ADDRESS, char* UID, struct addrinfo *res, int fd){
    char message[12], buffer[GROUPS];
    memset(buffer, 0, GROUPS);
    sprintf(message,"GLM %s\n", UID);
    if (udp_send_and_receive(fd, res, message, buffer, GROUPS) == -1)
        return;
    if (!strcmp("ERR\n", buffer)){
        puts(GEN_ERR);
        return;
    }  
    if (!strcmp("RGM E_USR\n",buffer)){
        puts(GRP_ERR);
        return;
    }
    char response[4], groups[3];
    memset(response, 0, 4);
    memset(groups, 0, 3);
    int format = sscanf(buffer, "%s %s", response, groups);
    if (!(format == 2 && !strcmp("RGM", response) && (strlen(groups) == 1 || strlen(groups) == 2) && digits_only(groups, "number of groups"))){
        puts(INFO_ERR);
        return;
    }
    printf("You are subscribed to %s %s:\n", groups, strcmp(groups, "1") ? "groups" : "group");
    show_groups(&(buffer[4+strlen(groups)]), groups);
}