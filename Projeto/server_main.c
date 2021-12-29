#include "server.h"

char PORT[6];
bool verbose = false;
int errcode;
struct addrinfo hints, *res;
struct sockaddr_in addr;
socklen_t addrlen;
ssize_t n , nread;
int udp_socket, tcp_socket;


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

int recv_udp(char* message){
    addrlen = sizeof(addr);
    nread = recvfrom(udp_socket, message, 128, 0, (struct sockaddr*)&addr, &addrlen);
    if (nread == -1){
        puts(RECV_ERR);
        return -1;
    }
    return nread;
}

int send_udp(char* message){
    addrlen = sizeof(addr);
    n = sendto(udp_socket, message, 9, 0, (struct sockaddr*)&addr,addrlen);
    if (n == -1){
        puts(SEND_ERR);
        return -1;
    }
    return n;
}

int socket_bind(int socktype){
    int sockfd = socket(AF_INET,socktype,0);
    if (sockfd == -1){
        puts(SOCK_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    hints.ai_flags= AI_PASSIVE;
    
    if (getaddrinfo(NULL, PORT, &hints, &res) != 0){
        puts(SOCK_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd,res->ai_addr,res->ai_addrlen) == -1){
        puts(SOCK_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int parse_argv(int argc, char* argv[]){
    if (argc < 1 || argc > 4 || strcmp(argv[0], "./DS"))
        return 0;
    memset(PORT, 0, 10);
    if (argc >= 2){
        if (!strcmp(argv[1], "-v")){
            verbose = true;
            if (argc > 2){
                if (!strcmp(argv[2], "-p") && digits_only(argv[3], "port number")){
                    strcpy(PORT, argv[3]);
                    return 1;
                }
                return 0;
            }
            strcpy(PORT, "58026");
            return 1;
        }
        if (!strcmp(argv[1], "-p") && argc > 2 && digits_only(argv[2], "port number")){
            strcpy(PORT, argv[2]);
            if (argc > 3){
                if (!strcmp(argv[3], "-v")){
                    verbose = true;
                    return 1;
                }
                return 0;
            }
            return 1;
        }
    }
    if (argc == 1){
        strcpy(PORT, "58026");
        return 1;
    }
    return 0;
}

void parse(){
    char message[BUF_SIZE];
    memset(message, 0, BUF_SIZE);
    if (recv_udp(message) == -1)
        exit(EXIT_FAILURE);

    char name[4];
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    memset(name, 0, 4);
    memset(arg1, 0, SIZE);
    memset(arg2, 0, SIZE);
    memset(arg3, 0, SIZE);
    if (sscanf(name, "%s ", message) < 1){
        puts(INVALID_CMD);
        return;
    }

    if (!strcmp(name, "PST")){
        //Post (TCP): "text" (Verificar as aspas, talvez?), [FName] (Verificar os parênteses, talvez?)  //e preciso mandar tbm o argumento 3 no caso do post, que vai ser o resto do text caso haja espacos
        /*int format = sscanf(command, "\"%[^\"]\" %s %[^\n]", arg1, arg2, arg3);
        if (strcmp(arg3, "")){
            puts("Wrong format! Too many arguments. Please try again!");
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
        //post(IP_ADDRESS, GID, UID, res, arg1, arg2);
        return;
    }*/
    if (!strcmp(name, "REG")){
        puts("Recebeu comando!");
        char teste[] = "funciona\n";
        send_udp(teste);
        //Register (UDP): UID (tam 5), pass (tam 8)
        //reg(IP_ADDRESS, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "UNR")){
        //Unegister (UDP): UID (tam 5), pass (tam 8)
        //unreg(IP_ADDRESS, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "LOG")){
        //Login (UDP): UID (tam 5), pass (tam 8)
        /*if (login(IP_ADDRESS, arg1, arg2, res, udp_socket) == 1){
            strcpy(UID, arg1);
            strcpy(password, arg2);
        }*/
    } else if (!strcmp(name, "OUT")){
        //Logout (UDP): (nada)
        /*if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        if (logout(IP_ADDRESS, UID, password, res, udp_socket) == 1)
            memset(UID, 0, 6);*/
    } /*else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))                   //o server nao recebe mensagem de exit (?)
            return;
        exit(EXIT_SUCCESS);*/
    } else if (!strcmp(name, "GLS")){
        //Groups (UDP): (nada)
        /*if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        groups(IP_ADDRESS, res, udp_socket);*/
    } else if (!strcmp(name, "GSR")){
        //Subscribe (UDP): GID (tam 2), GName (tam 24)
        //subscribe(IP_ADDRESS, UID, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "GUR")){
        //Unsubscribe (UDP): GID (tam 2)
        /*if (!(has_correct_arg_sizes(arg1, 2, arg2, 0) && digits_only(arg1, "GID")))
            return;
        unsubscribe(IP_ADDRESS, UID, arg1, res, udp_socket);*/
    } else if (!strcmp(name, "GLM")){
        //My groups (UDP): (nada)
        /*if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        my_groups(IP_ADDRESS, UID, res, udp_socket);*/
    } else if (!strcmp(name, "ULS")){
        //User list (TCP): (nada)
        /*if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(UID) && check_select(GID)))
            return;
        ulist(IP_ADDRESS, GID, res);*/
    } else if (!strcmp(name, "RTV")){
        //Retrieve (TCP): MID
        /*if (!(has_correct_arg_sizes(arg1, 4, arg2, 0) && digits_only(arg1, "message ID")))
            return;
        retrieve(IP_ADDRESS, GID, UID, arg1, res);*/
    } else
        puts(INVALID_CMD);
    puts("----------------------------------------"); //repetir em todos os ifs

}

int main(int argc, char* argv[]){
    if (!parse_argv(argc, argv)){           // ./DS -v -p 58026 funciona com este, algumas verificacoes nao estao bem alteradas
        puts(ARGV_ERR);
        exit(EXIT_FAILURE);
    }

    //Criacao e bind dos sockets udp e tcp do servidor
    int udp_socket = socket_bind(SOCK_DGRAM);
    int tcp_socket = socket_bind(SOCK_STREAM);
    while (1){
        //ve se tem uma nova coneccao udp ou tcp com o select() !!
        //select();
        
        
        //NOTA:
        //a mensagem udp nao pode ser separada por isso é preciso recebe-la inteira e dps tratar da string
        //se calhar o melhor approach seria ter uma funcao para receber a informacao (recvfrom) e outra para enviar a resposta depois de aplicar a funcao respetiva (com o sendto) 
        parse();  //teste simplesmente com uma ligacao udp

    }
    //memset(command, 0, SIZE);
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}