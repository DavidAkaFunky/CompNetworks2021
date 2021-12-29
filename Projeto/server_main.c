#include "server.h"

char PORT[6];
bool verbose = false;
int udp_socket, tcp_socket;
struct addrinfo hints, *res;
struct sockaddr_in serv_addr, cli_addr;
socklen_t addrlen;
ssize_t n, nread;


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
    addrlen = sizeof(serv_addr);
    nread = recvfrom(udp_socket, message, 128, 0, (struct sockaddr*)&serv_addr, &addrlen);
    if (nread == -1){
        puts(RECV_ERR);
        return -1;
    }
    return nread;
}

int send_udp(char* message){
    addrlen = sizeof(serv_addr);
    n = sendto(udp_socket, message, 9, 0, (struct sockaddr*)&serv_addr,addrlen);
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
    bzero(&hints, sizeof hints);
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
    bzero(PORT, 6);
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

void parse(char* message){
    if (recv_udp(message) == -1)
        exit(EXIT_FAILURE);
    char name[4];
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    bzero(name, 4);
    bzero(arg1, SIZE);
    bzero(arg2, SIZE);
    bzero(arg3, SIZE);
    if (sscanf(message, "%s ", name) < 1){
        puts(INVALID_CMD);
        return;
    }
    message += strlen(name) + 1;
    puts(name);
    /*if (!strcmp(name, "PST")){
        //Post (TCP): "text" (Verificar as aspas, talvez?), [FName] (Verificar os parênteses, talvez?)  //e preciso mandar tbm o argumento 3 no caso do post, que vai ser o resto do text caso haja espacos
        int format = sscanf(command, "\"%[^\"]\" %s %[^\n]", arg1, arg2, arg3);
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
            bzero(UID, 6);*/
    } /*else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))                   //o server nao recebe mensagem de exit (?)
            return;
        exit(EXIT_SUCCESS);*/
    else if (!strcmp(name, "GLS")){
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
    udp_socket = socket_bind(SOCK_DGRAM);
    tcp_socket = socket_bind(SOCK_STREAM);
    listen(tcp_socket, 10);
    char message[BUF_SIZE];
    fd_set rset;
    int conn_fd;
    
    // clear the descriptor set
    FD_ZERO(&rset);
 
    // get maxfd
    int maxfd = udp_socket > tcp_socket ? udp_socket + 1 : tcp_socket + 1;
    while (1) {
 
        // set tcp_socket and udp_socket in readset
        FD_SET(tcp_socket, &rset);
        FD_SET(udp_socket, &rset);
 
        // select the ready descriptor
        int nready = select(maxfd, &rset, NULL, NULL, NULL);
 
        // if tcp socket is readable then handle
        // it by accepting the connection
        if (FD_ISSET(tcp_socket, &rset)) {
            socklen_t len = sizeof(cli_addr);
            conn_fd = accept(tcp_socket, (struct sockaddr*)&cli_addr, &len);
            close(tcp_socket);
            bzero(message, sizeof(message));
            recv_tcp(message);
            if (verbose){
                printf("Message from TCP client:\n%s\n", message);
            }
            send_tcp(message);
            close(conn_fd);
        }
        // if udp socket is readable receive the message.
        if (FD_ISSET(udp_socket, &rset)) {
            bzero(message, sizeof(message));
            recv_udp(message);
            if (verbose){
                printf("Message from UDP client:\n%s\n", message);
            }
            send_udp(message);
        }
        
    }
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}