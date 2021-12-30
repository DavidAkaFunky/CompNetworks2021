#include "client.h" 

char IP_ADDRESS[512], PORT[6];
struct addrinfo hints, *res;

int is_alphanumerical(char* s, int flag){
    while (*s) {
        if (!(isalpha(*s) || isdigit(*s))){
            switch (flag){
                case 0:
                    if(!(*s == 32)){
                        puts(NO_ALPH0);
                        return 0;
                    }
                    break;
                case 1:
                    if(!(*s == 45 || *s == 95)){
                        puts(NO_ALPH1);
                        return 0;
                    }
                    break;
                case 2:
                    if(!(*s == 45 || *s == 46 || *s == 95)){
                        puts(NO_ALPH2);
                        return 0;
                    }
                    break;
            }
        }
        s++;
    }
    return 1;
}

int is_correct_arg_size(char* arg, int size){
    if (strlen(arg) != size){
        printf("%s's size is not %d. Please try again!\n", arg, size);
        return 0;
    }
    return 1;
}

int has_correct_arg_sizes(char* arg1, int size1, char* arg2, int size2){
    return is_correct_arg_size(arg1, size1) && is_correct_arg_size(arg2, size2);
}

int digits_only(char *s, char* id){
    while (*s) {
        if (!isdigit(*s)){
            printf("The %s has a non-numeric character. Please try again!\n", id);
            return 0;
        }
        s++;
    }
    return 1;
}

int check_login(char *UID){
    if (strlen(UID) != 5){
        puts(NO_LOGIN);
        return 0;
    }
    return 1;
}

int check_select(char *GID){
    if (strlen(GID) != 2){
        puts(NO_GROUP);
        return 0;
    }
    return 1;
}

int create_socket(int socktype){
    int sockfd = socket(AF_INET,socktype,0);
    if (sockfd == -1){
        puts(SOCK_FAIL);
        exit(EXIT_FAILURE);
    }
    bzero(&hints, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    if(getaddrinfo(IP_ADDRESS, PORT, &hints, &res) != 0){
        puts(ADDR_FAIL);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

//Esta função provavelmente é inútil - Perguntar aos profs!!!
int get_IP(){
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

int get_local_IP(){
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

int parse_argv(int argc, char* argv[]){
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
            return get_local_IP();
        }
    }
    
    if (argc == 1){
        strcpy(PORT, "58026");
        return get_local_IP();
    }
    return 0;
}

void parse(int udp_socket, char* command, char* UID, char* password, char* GID){
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
            puts(ERR_FORMAT);
            return;
        }
        if (format == 0){
            puts(NO_TEXT);
            return;
        }
        post(IP_ADDRESS, GID, UID, res, arg1, arg2);
        return;
    }
    sscanf(command, "%s %s %[^\n]", arg1, arg2, arg3);
    if (strcmp(arg3, "")){ //Isto deve falhar se tivermos espaços depois do arg2? (Perguntar ao prof) //ALTERADO com a mensagem do post pode haver espacos por isso o arg 3 fica obsoleto
        puts("Too many arguments. Please try again!");
        return;
    }
    if (!strcmp(name, "reg")){
        //Register (UDP): UID (tam 5), pass (tam 8)
        reg(IP_ADDRESS, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "unregister") || !strcmp(name, "unr")){
        //Unegister (UDP): UID (tam 5), pass (tam 8)
        unreg(IP_ADDRESS, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "login")){
        //Login (UDP): UID (tam 5), pass (tam 8)
        if (login(IP_ADDRESS, arg1, arg2, res, udp_socket) == 1){
            strcpy(UID, arg1);
            strcpy(password, arg2);
        }
    } else if (!strcmp(name, "logout")){
        //Logout (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        if (logout(IP_ADDRESS, UID, password, res, udp_socket) == 1)
            bzero(UID, 6);
    } else if (!strcmp(name, "showuid") || !strcmp(name, "su")){
        //displays UID : (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(UID)))
            return;
        printf("The UID selected is %s.\n",UID);
    } else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        freeaddrinfo(res);
        exit(EXIT_SUCCESS);
    } else if (!strcmp(name, "groups") || !strcmp(name, "gl")){
        //Groups (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        groups(IP_ADDRESS, res, udp_socket);
    } else if (!strcmp(name, "subscribe") || !strcmp(name, "s")){
        //Subscribe (UDP): GID (tam 2), GName (tam 24)
        subscribe(IP_ADDRESS, UID, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "unsubscribe") || !strcmp(name, "u")){
        //Unsubscribe (UDP): GID (tam 2)
        if (!(has_correct_arg_sizes(arg1, 2, arg2, 0) && digits_only(arg1, "GID")))
            return;
        unsubscribe(IP_ADDRESS, UID, arg1, res, udp_socket);
    } else if (!strcmp(name, "my_groups") || !strcmp(name, "mgl")){
        //My groups (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        my_groups(IP_ADDRESS, UID, res, udp_socket);
    } else if (!strcmp(name, "select") || !strcmp(name, "sag")){
        //Select: GID (tam 2)
        if (!(has_correct_arg_sizes(arg1, 2, arg2, 0) && digits_only(arg1,"GID") && check_login(UID)))
            return;
        strcpy(GID, arg1);
        printf("Group %s sucessfully selected.\n", GID);
    } else if (!strcmp(name, "showgid") || !strcmp(name, "sg")){
        //displays selected group : (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(UID) && check_select(GID)))
            return;
        printf("The group selected is %s.\n", GID);
    } else if (!strcmp(name, "ulist") || !strcmp(name, "ul")){
        //User list (TCP): (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(UID) && check_select(GID)))
            return;
        ulist(IP_ADDRESS, GID, res);
    } else if (!strcmp(name, "retrieve") || !strcmp(name, "r")){
        //Retrieve (TCP): MID
        if (!(has_correct_arg_sizes(arg1, 4, arg2, 0) && digits_only(arg1, "message ID")))
            return;
        retrieve(IP_ADDRESS, GID, UID, arg1, res);
    } else
        puts(INVALID_CMD);
}

int main(int argc, char* argv[]){
    char command[SIZE], UID[6], password[9], GID[3];
    if (!parse_argv(argc, argv)){
        puts(ARGV_ERR);
        exit(EXIT_FAILURE);
    }
    int udp_socket = create_socket(SOCK_DGRAM);
    bzero(UID, 6);
    bzero(password, 9);
    bzero(GID, 3);
    while(fgets(command, SIZE, stdin)){
        parse(udp_socket, command, UID, password, GID);
        bzero(command, SIZE);
        puts("----------------------------------------");
    }
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}