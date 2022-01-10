#include "client.h" 
#include "../common.h"

/**
 * @brief Activates timeout limit (given by the constant TIME) for client sockets.
 * 
 * @param[in] sd the socket whose timer will be activated.
 * @param[in] time the number of seconds to wait for a reply from the server.
 * @return 0 if the activation using setsockopt was successful, -1 otherwise.
 */
int timer_on(int sd, int time){
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = time;
    return setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval));
}

/**
 * @brief Deactivates timeout limit for client sockets.
 * 
 * @param[in] sd the socket whose timer will be deactivated.
 * @return 0 if the deactivation using setsockopt was successful, -1 otherwise.
 */
int timer_off(int sd){
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure */
    return setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tmout,sizeof(struct timeval));
}

/**
 * @brief Sends and receives the messages between the server and the client (UDP).
 * 
 * @param fd the udp socket used to communicate.
 * @param res information about the address of the service provider.
 * @param message message sent from the client to the server.
 * @param buffer message received from the server.
 * @param size the size of the message received from the server.
 * @return the quantity of bytes read.
 */
int udp_send_and_receive(int fd, struct addrinfo *res, char* message, char* buffer, int size){
    ssize_t bytes = sendto(fd, message, strlen(message), 0, res -> ai_addr, res -> ai_addrlen);
	if (bytes == -1){
        puts(SEND_ERR);
        return -1;
    }
    struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
    int i = 0;
    while (true){
        timer_on(fd, 2);
        bytes = recvfrom(fd, buffer, size, 0, (struct sockaddr*) &addr, &addrlen);
        timer_off(fd);
        if (bytes == -1){
            if ((++i) == 3){
                puts(RECV_ERR);
                return -1;
            }
            puts(RECV_NEW_ATT);
        }
        else
            return bytes;
    }
}

/**
 * @brief Executes the register command.
 * 
 * @param uid the user ID to be registered.
 * @param password the password of the user to be registered.
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 */
void reg(char* uid, char* password, struct addrinfo *res, int fd){
    if (!(digits_only(uid,"uid") && has_correct_arg_sizes(uid, 5, password, 8) && is_alphanumerical(password, 0)))
        return;
    char message[20], buffer[BUF_SIZE];
    bzero(message, 20);
    bzero(buffer, BUF_SIZE);
    sprintf(message,"REG %s %s\n",uid,password);
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

/**
 * @brief Executes the unregister command.
 * 
 * @param uid the user ID to be unregistered.
 * @param password the password of the user to be unregistered.
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 */
void unreg(char* uid, char* password, struct addrinfo *res, int fd){
    if (!(digits_only(uid,"uid") && has_correct_arg_sizes(uid, 5, password, 8) && is_alphanumerical(password, 0)))
        return;
    char message[20], buffer[BUF_SIZE];
    bzero(message, 20);
    bzero(buffer, BUF_SIZE);
    sprintf(message,"UNR %s %s\n", uid, password);
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

/**
 * @brief Executes the login command.
 * 
 * @param uid the user ID to be logged in.
 * @param password the password of the user to be logged in.
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 * @return the value that indicates success or failure.
 */
int login(char* uid, char* password, struct addrinfo *res, int fd){
    if (!(digits_only(uid,"uid") && has_correct_arg_sizes(uid, 5, password, 8) && is_alphanumerical(password, 0)))
        return -1;
    char message[20], buffer[BUF_SIZE];
    bzero(message, 20);
    bzero(buffer, BUF_SIZE);
    sprintf(message,"LOG %s %s\n",uid,password);
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

/**
 * @brief Executes the logout command.
 * 
 * @param uid the user ID to be logged out.
 * @param password the password of the user to be logged out.
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 * @return the value that indicates success or failure.
 */
int logout(char* uid, char* password, struct addrinfo *res, int fd){
    char message[20], buffer[BUF_SIZE];
    bzero(message, 20);
    bzero(buffer, BUF_SIZE);
    sprintf(message,"OUT %s %s\n", uid, password);
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

/**
 * @brief Auxiliary function that prints the groups to stdout
 * 
 * @param ptr ptr to the buffer that has every Group with the gid, group name and mid, in this specific order
 * @param groups string that has the number of groups
 */
void show_groups(char* ptr, char* groups){
    int n = atoi(groups), format;
    char group_name[25];
    char mid[5];
    for (int i = 0; i < n; ++i){
        bzero(groups, 3);
        bzero(group_name, 25);
        bzero(mid, 5);
        format = sscanf(ptr, " %s %s %s", groups, group_name, mid);
        if (!(format == 3 && has_correct_arg_sizes(groups, 2, mid, 4) && digits_only(groups, "gid") && strlen(group_name) <= 24 && is_alphanumerical(group_name, 1) && digits_only(mid, "MID"))){
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

/**
 * @brief Executes the groups command.
 *
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 */
void groups(struct addrinfo *res, int fd){
    char buffer[GROUPS];
    bzero(buffer, GROUPS);
    if (udp_send_and_receive(fd, res, "GLS\n", buffer, GROUPS) == -1)
        return;
    char response[4], groups[3];
    bzero(response, 4);
    bzero(groups, 3);
    if (!strcmp("ERR\n", buffer)){
        puts(GEN_ERR);
        return;
    }
    int format = sscanf(buffer, "%s %s", response, groups);
    if (!(format == 2 && !strcmp("RGL", response) && (strlen(groups) == 1 || strlen(groups) == 2) && digits_only(groups, "number of groups"))){
        puts(INFO_ERR);
        return;
    }
    if (!strcmp(groups, "0")){
        puts(NO_GROUPS);
    }
    else{
        printf("There %s %s registered %s:\n", strcmp(groups, "1") ? "are" : "is", groups, strcmp(groups, "1") ? "groups" : "group");
        show_groups(&(buffer[4+strlen(groups)]), groups);
    }
}

/**
 * @brief Executes the subscribe command.
 * 
 * @param uid the user ID to be subscribed.
 * @param gid the group ID to subscribe.
 * @param group_name the group name to subscribe.
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 */
void subscribe(char* uid, char* gid, char* group_name, struct addrinfo *res, int fd){
    if (strlen(gid) == 1)
        sprintf(gid, "0%c", gid[0]);
    if (!(has_correct_arg_sizes(uid, 5, gid, 2) && digits_only(uid,"uid") && digits_only(gid,"gid") && strlen(group_name) <= 24 && is_alphanumerical(group_name, 1) && strlen(group_name) >= 1)){
        puts("One or more arguments are invalid. Please try again!");
        return;
    }
    char message[38], buffer[BUF_SIZE];
    bzero(message, 38);
    bzero(buffer, BUF_SIZE);
    sprintf(message,"GSR %s %s %s\n", uid, gid, group_name);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
    if (!strcmp("ERR\n", buffer))
        puts(GEN_ERR);
    else if (!strcmp("RGS OK\n",buffer)) 
        printf("You have successfully subscribed to group %s!\n", gid);
    else if (!strcmp("RGS E_GRP\n",buffer))
        puts(GRP_FAIL);
    else if (!strcmp("RGS E_GNAME\n",buffer))
        puts(REG_GRP_INV);
    else if (!strcmp("RGS E_FULL\n",buffer))
        puts(REG_GRP_FULL);
    else if (!strcmp("RGS NOK\n",buffer)) 
        puts(REG_GRP_ERR1);
    else {
        char cmd1[4], cmd2[4], new_gid[3], extra[SIZE];
        bzero(cmd1, 4);
        bzero(cmd2, 4);
        bzero(new_gid, 3);
        bzero(extra, SIZE);
        int format = sscanf(buffer, "%s %s %s %s", cmd1, cmd2, new_gid, extra);
        if (!(format == 3 && !strcmp(cmd1, "RGS") && !strcmp(cmd1, "NEW") && is_correct_arg_size(new_gid, 2) && !strcmp(extra, ""))){
            strcpy(gid, new_gid);
            printf("New group created with gid %s!\n", new_gid);
        }
        else
            puts(INFO_ERR);
    }
}

/**
 * @brief Executes the unsubscribe command.
 * 
 * @param uid the user ID to be unsubscribed.
 * @param gid the group ID to unsubscribe.
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 */
void unsubscribe(char* uid, char* gid, struct addrinfo *res, int fd) {
    char message[13], buffer[BUF_SIZE];
    sprintf(message,"GUR %s %s\n", uid, gid);
    if (udp_send_and_receive(fd, res, message, buffer, BUF_SIZE) == -1)
        return;
    if (!strcmp("ERR\n", buffer))
        puts(GEN_ERR);
    else if (!strcmp("RGU OK\n",buffer)) 
        printf("You have successfully unsubscribed from group %s.\n", gid);
    else if (!strcmp("RGU E_USR\n",buffer))
        puts(UNR_GRP_FAIL_USR);
    else if (!strcmp("RGU E_GRP\n",buffer))
        puts(GRP_FAIL);
    else if (!strcmp("RGU NOK\n",buffer)) 
        puts(UNR_GRP_ERR1);
    else
        puts(INFO_ERR);
}

/**
 * @brief Executes the my_groups command.
 * 
 * @param uid the user ID to show the groups he is subscribed.
 * @param res information about the address of the service provider.
 * @param fd the udp socket used to communicate.
 */
void my_groups(char* uid, struct addrinfo *res, int fd){
    char message[12], buffer[GROUPS];
    bzero(buffer, GROUPS);
    sprintf(message,"GLM %s\n", uid);
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
    bzero(response, 4);
    bzero(groups, 3);
    int format = sscanf(buffer, "%s %s", response, groups);
    if (!(format == 2 && !strcmp("RGM", response) && (strlen(groups) == 1 || strlen(groups) == 2) && digits_only(groups, "number of groups"))){
        puts(INFO_ERR);
        return;
    }
    printf("You are subscribed to %s %s:\n", groups, strcmp(groups, "1") ? "groups" : "group");
    show_groups(&(buffer[4+strlen(groups)]), groups);
}