#include "client.h" 

char IP_ADDRESS[20], PORT[20];
int errcode;
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
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    if(getaddrinfo(IP_ADDRESS, PORT, &hints, &res) != 0){
        puts(ADDR_FAIL);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void parse(int udp_socket, char* command, char* UID, char* password, char* GID){
    char name[12]; //The largest command name has 11 characters '\0'
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    memset(name, 0, 12);
    memset(arg1, 0, SIZE);
    memset(arg2, 0, SIZE);
    memset(arg3, 0, SIZE);
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
            memset(UID, 0, 6);
    } else if (!strcmp(name, "showuid") || !strcmp(name, "su")){
        //displays UID : (nada)
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(UID)))
            return;
        printf("The UID selected is %s.\n",UID);
    } else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
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
        if (!has_correct_arg_sizes(arg1, 0/*???*/, arg2, 0))
            return;
    } else
        puts(INVALID_CMD);
}

int main(int argc, char* argv[]){
    char command[SIZE], UID[6], password[9], GID[3];
    //Fazer parse do argv e ver se os argumentos existem
    strcpy(IP_ADDRESS,argv[2]);         //Defines the IP_ADDRESS where the server runs
    strcpy(PORT,argv[4]);               //Defines the PORT where the server accepts requests
    int udp_socket = create_socket(SOCK_DGRAM);
    memset(UID, 0, 6);
    memset(password, 0, 9);
    memset(GID, 0, 3);
    while(fgets(command, SIZE, stdin)){
        parse(udp_socket, command, UID, password, GID);
        memset(command, 0, SIZE);
    }
}