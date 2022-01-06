#include "client.h"
#include "../common.h"
#include <sys/types.h>
#include <sys/socket.h>

int TimerON(int sd){
    struct timeval tmout;
    
    memset((char *)&tmout,0,sizeof(tmout)); /* clear time structure */
    tmout.tv_sec=3; /* Wait for 15 sec for a reply from server. */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,
    (struct timeval *)&tmout,sizeof(struct timeval)));
}


int TimerOFF(int sd){
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* clear time structure */
    return(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,
    (struct timeval *)&tmout,sizeof(struct timeval)));
}


int check_login(char *uid, bool log){
    if (strlen(uid) != 5 && log){
        puts(NO_LOGIN);
        return 0;
    }
    else if (strlen(uid) != 5 && !log)
        return 0;
    return 1;
}

int check_select(char *gid){
    if (strlen(gid) != 2){
        puts(NO_GROUP);
        return 0;
    }
    return 1;
}

int create_socket(struct addrinfo **res, int socktype, char* IP_ADDRESS, char* PORT){
    int sockfd = socket(AF_INET,socktype,0);
    if (sockfd == -1){
        puts(SOCK_FAIL);
        exit(EXIT_FAILURE);
    }
    struct addrinfo hints;
    bzero(&hints, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    if(getaddrinfo(IP_ADDRESS, PORT, &hints, res) != 0){
        puts(ADDR_FAIL);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

//Esta função provavelmente é inútil - Perguntar aos profs!!!
int get_IP(char* IP_ADDRESS){
    char part1[20], part2[4], part3[4], part4[4];
    bzero(part1, 4);
    bzero(part2, 4);
    bzero(part3, 4);
    bzero(part4, 4);
    if (sscanf(IP_ADDRESS, "%[^.].%[^.].%[^.].%[^.]", part1, part2, part3, part4) == 4 &&
        0 < atoi(part1) && atoi(part1) < 255 && 0 < atoi(part2) && atoi(part2) < 255 &&
        0 < atoi(part3) && atoi(part3) < 255 && 0 < atoi(part4) && atoi(part4) < 255)
            return strlen(part1) <= 3 && strlen(part2) <= 3 && strlen(part3) <= 3 && strlen(part4) <= 3;
            
    return 1;
}

int get_local_IP(char* IP_ADDRESS){
    char hostbuffer[256];
    
    int hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    if (hostname == -1)
        return 0;

    struct hostent *host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL)
        return 0;
    
    strcpy(IP_ADDRESS, inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])));
    return IP_ADDRESS != NULL;
}

int parse_argv(char* IP_ADDRESS, char* PORT, int argc, char** argv){
    if (!(argc == 1 || argc == 3 || argc == 5) || strcmp(argv[0], "./user"))
        return 0;
    bzero(IP_ADDRESS, 512);
    bzero(PORT, 6);
    if (argc >= 3){
        if (!strcmp(argv[1], "-n")){
            strcpy(IP_ADDRESS, argv[2]);
            /*if (!get_IP())
                return 0;*/
            if (argc > 3){
                if (!strcmp(argv[3], "-p") && digits_only(argv[4], "port number")){
                    strcpy(PORT, argv[4]);
                    return 1;
                }
                return 0;
            }
            strcpy(PORT, "58026");
            return 1;
        }
        if (!strcmp(argv[1], "-p") && digits_only(argv[2], "port number")){
            strcpy(PORT, argv[2]);
            if (argc > 3){
                if (!strcmp(argv[3], "-n")){
                    strcpy(IP_ADDRESS, argv[4]);
                    return 1; /*get_IP();*/
                }
                return 0;
            }
            return get_local_IP(IP_ADDRESS);
        }
    }
    
    if (argc == 1){
        strcpy(PORT, "58026");
        return get_local_IP(IP_ADDRESS);
    }
    return 0;
}


void parse(int udp_socket, int tcp_socket, struct addrinfo *res, char* IP_ADDRESS, char* PORT, char* command, char* uid, char* password, char* gid){
    char name[12]; //The largest command name has 11 characters '\0'
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    bzero(name, 12);
    bzero(arg1, SIZE);
    bzero(arg2, SIZE);
    bzero(arg3, SIZE);
    if (sscanf(command, "%s ", name) < 1){
        puts(INVALID_CMD);
        return;
    }
    command += strlen(name) + 1;
    if (!strcmp(name, "post")){
        //Post (TCP): "text" (Verificar as aspas, talvez?), [FName] (Verificar os parênteses, talvez?)  //e preciso mandar tbm o argumento 3 no caso do post, que vai ser o resto do text caso haja espacos
        int format = sscanf(command, "\"%[^\"]\" %s %[^\n]", arg1, arg2, arg3);
        if (strcmp(arg3, "")){
            puts("Too many arguments. Please try again!");
            return;
        }
        if (format == -1){
            puts(FORMAT_ERR);
            return;
        }
        if (format == 0){
            puts(NO_TEXT);
            return;
        }
        post(IP_ADDRESS, PORT, gid, uid, res, arg1, arg2, tcp_socket);
        close(tcp_socket);
        return;
    }
    sscanf(command, "%s %s %[^\n]", arg1, arg2, arg3);
    if (strcmp(arg3, "")){ //Isto deve falhar se tivermos espaços depois do arg2? (Perguntar ao prof) //ALTERADO com a mensagem do post pode haver espacos por isso o arg 3 fica obsoleto
        puts("Too many arguments. Please try again!");
        return;
    }
    if (!strcmp(name, "reg")){
        //Register (UDP): uid (tam 5), pass (tam 8)
        reg(IP_ADDRESS, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "unregister") || !strcmp(name, "unr")){
        //Unegister (UDP): uid (tam 5), pass (tam 8)
        unreg(IP_ADDRESS, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "login")){
        //Login (UDP): uid (tam 5), pass (tam 8)
        if (!check_login(uid, false)){
            if (login(IP_ADDRESS, arg1, arg2, res, udp_socket) == 1){
                strcpy(uid, arg1);
                strcpy(password, arg2);
            }
            return;
        }
        puts(LOGIN_DOUBLE);
    } else if (!strcmp(name, "logout")){
        //Logout (UDP): (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && is_correct_arg_size(uid, 5)))
            return;
        if (logout(IP_ADDRESS, uid, password, res, udp_socket) == 1)
            bzero(uid, 6);
    } else if (!strcmp(name, "showuid") || !strcmp(name, "su")){
        //displays uid : (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true)))
            return;
        printf("The uid selected is %s.\n",uid);
    } else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        freeaddrinfo(res);
        logout(IP_ADDRESS, uid, password, res, udp_socket);
        exit(EXIT_SUCCESS);
    } else if (!strcmp(name, "groups") || !strcmp(name, "gl")){
        //Groups (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        groups(IP_ADDRESS, res, udp_socket);
    } else if (!strcmp(name, "subscribe") || !strcmp(name, "s")){
        //Subscribe (UDP): gid (tam 2), group_name (tam 24)
        subscribe(IP_ADDRESS, uid, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "unsubscribe") || !strcmp(name, "u")){
        //Unsubscribe (UDP): gid (tam 2)
        if (!(has_correct_arg_sizes(arg1, 2, arg2, 0) && digits_only(arg1, "gid")))
            return;
        unsubscribe(IP_ADDRESS, uid, arg1, res, udp_socket);
    } else if (!strcmp(name, "my_groups") || !strcmp(name, "mgl")){
        //My groups (UDP): (nada)
        if (!(check_login(uid, true) && has_correct_arg_sizes(arg1, 0, arg2, 0)))
            return;
        my_groups(IP_ADDRESS, uid, res, udp_socket);
    } else if (!strcmp(name, "select") || !strcmp(name, "sag")){
        //Select: gid (tam 2)
        if (!(has_correct_arg_sizes(arg1, 2, arg2, 0) && digits_only(arg1,"gid") && check_login(uid, true)))
            return;
        strcpy(gid, arg1);
        printf("Group %s sucessfully selected.\n", gid);
    } else if (!strcmp(name, "showgid") || !strcmp(name, "sg")){
        //displays selected group : (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true) && check_select(gid)))
            return;
        printf("The group selected is %s.\n", gid);
    } else if (!strcmp(name, "ulist") || !strcmp(name, "ul")){
        //User list (TCP): (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true) && check_select(gid)))
            return;
        ulist(IP_ADDRESS, PORT, gid, res, tcp_socket);
        close(tcp_socket);
    } else if (!strcmp(name, "retrieve") || !strcmp(name, "r")){
        //Retrieve (TCP): MID
        if (!(has_correct_arg_sizes(arg1, 4, arg2, 0) && digits_only(arg1, "message ID")))
            return;
        retrieve(IP_ADDRESS, PORT, gid, uid, arg1, res, tcp_socket);
        close(tcp_socket);
    } else
        puts(INVALID_CMD);
}

int main(int argc, char** argv){
    char command[SIZE], uid[6], password[9], gid[3], IP_ADDRESS[512], PORT[6];
    if (!parse_argv(IP_ADDRESS, PORT, argc, argv)){
        puts(ARGV_ERR);
        exit(EXIT_FAILURE);
    }
    struct addrinfo *res;
    int udp_socket = create_socket(&res, SOCK_DGRAM, IP_ADDRESS, PORT), tcp_socket;
    bzero(uid, 6);
    bzero(password, 9);
    bzero(gid, 3);
    if (mkdir("DOWNLOADS", 0700) == -1 && access("DOWNLOADS", F_OK)){
        puts(DOWNLOADS_FAIL);
        exit(EXIT_FAILURE);
    }
    while(fgets(command, SIZE, stdin)){
        parse(udp_socket, tcp_socket, res, IP_ADDRESS, PORT, command, uid, password, gid);
        bzero(command, SIZE);
        puts("----------------------------------------");
    }
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}