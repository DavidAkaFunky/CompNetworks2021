#include "server.h"
#include "../common.h"

struct sockaddr_in serv_addr;


int recv_udp(int udp_socket, char* message){
    socklen_t addrlen = sizeof(serv_addr);
    ssize_t nread = recvfrom(udp_socket, message, 128, 0, (struct sockaddr*)&serv_addr, &addrlen);
    if (nread == -1){
        puts(RECV_ERR);
        return -1;
    }
    return nread;
}

int send_udp(int udp_socket, char* message){
    printf("Message: %s\n", message);
    socklen_t addrlen = sizeof(serv_addr);
    ssize_t n = sendto(udp_socket, message, strlen(message), 0, (struct sockaddr*)&serv_addr,addrlen);
    if (n == -1){
        puts(SEND_ERR);
        return -1;
    }
    return n;
}

int recv_tcp(int tcp_socket, char* message){
    return -1;
}

int send_tcp(int tcp_socket, char* message){
    return -1;
}

int socket_bind(int socktype, char* PORT, struct addrinfo** res){
    int sockfd = socket(AF_INET,socktype,0);
    if (sockfd == -1){
        puts(SOCK_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    struct addrinfo hints;
    bzero(&hints, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    hints.ai_flags= AI_PASSIVE;
    
    if (getaddrinfo(NULL, PORT, &hints, res) != 0){
        puts(SOCK_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd,(*res)->ai_addr,(*res)->ai_addrlen) == -1){
        puts(SOCK_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int parse_argv(int argc, char** argv, char* PORT, bool* verbose){
    if (argc < 1 || argc > 4 || strcmp(argv[0], "./DS"))
        return 0;
    bzero(PORT, 6);
    if (argc >= 2){
        if (!strcmp(argv[1], "-v")){
            *verbose = true;
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
                    *verbose = true;
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

int parse(int udp_socket, char* message, char* response){
    char name[4];
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    char arg4[SIZE];
    bzero(name, 4);
    bzero(arg1, SIZE);
    bzero(arg2, SIZE);
    bzero(arg3, SIZE);
    bzero(arg4, SIZE);
    sscanf(message, "%s %s %s %s %[^\n]", name, arg1, arg2, arg3, arg4);
    if (strcmp(arg4, "")){
        puts(INVALID_CMD);
        return 0;
    }
    if (!strcmp(name, "REG")){        
        //Register (UDP): uid (tam 5), pass (tam 8)
        return !strcmp(arg3, "") && reg(udp_socket, arg1, arg2);
    } else if (!strcmp(name, "UNR")){
        //Unegister (UDP): uid (tam 5), pass (tam 8)
        return !strcmp(arg3, "") && unreg(udp_socket, arg1, arg2);
    } else if (!strcmp(name, "LOG")){
        //Login (UDP): uid (tam 5), pass (tam 8)
        return !strcmp(arg3, "") && login(udp_socket, arg1, arg2);
    } else if (!strcmp(name, "OUT")){
        //Logout (UDP): (nada)
        return !strcmp(arg3, "") && logout(udp_socket, arg1,arg2);
    } /*else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))                   //o server nao recebe mensagem de exit (?)
            return;
        exit(EXIT_SUCCESS);*/
    else if (!strcmp(name, "GLS")){
        //Groups (UDP): (nada)
        return !strcmp(arg1, "") && groups(udp_socket);
    } else if (!strcmp(name, "GSR")){
        //Subscribe (UDP): gid (tam 2), group_name (tam 24)
        return subscribe(udp_socket, arg1, arg2, arg3);
    } else if (!strcmp(name, "GUR")){
        //Unsubscribe (UDP): gid (tam 2)
        return unsubscribe(udp_socket, arg1, arg2);
    } else if (!strcmp(name, "GLM")){
        //My groups (UDP): (nada)
        return !strcmp(arg2, "") && my_groups(udp_socket, arg1);
    } else if (!strcmp(name, "ULS")){
        //User list (TCP): (nada)
        /*if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid) && check_select(gid)))
            return;
        ulist(IP_ADDRESS, gid, res);*/
    } else if (!strcmp(name, "RTV")){
        //Retrieve (TCP): MID
        /*if (!(has_correct_arg_sizes(arg1, 4, arg2, 0) && digits_only(arg1, "message ID")))
            return;
        retrieve(IP_ADDRESS, gid, uid, arg1, res);*/
    } else
        puts(INVALID_CMD);
}

int main(int argc, char** argv){
    bool verbose = false;
    char PORT[6];
    if (!parse_argv(argc, argv, PORT, &verbose)){           // ./DS -v -p 58026 funciona com este, algumas verificacoes nao estao bem alteradas
        puts(ARGV_ERR);
        exit(EXIT_FAILURE);
    }
    if (mkdir("USERS", 0700) == -1 && access("USERS", F_OK)){
        puts(USERS_FAIL);
        exit(EXIT_FAILURE);
    }
    if (mkdir("GROUPS", 0700) == -1 && access("GROUPS", F_OK)){
        puts(GROUPS_FAIL);
        exit(EXIT_FAILURE);
    }
    
    //Criacao e bind dos sockets udp e tcp do servidor
    struct addrinfo *res;
    int udp_socket = socket_bind(SOCK_DGRAM, PORT, &res);
    int tcp_socket = socket_bind(SOCK_STREAM, PORT, &res);
    listen(tcp_socket, 10);
    char message[BUF_SIZE], response[BUF_SIZE];
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
            socklen_t len = sizeof(serv_addr);
            conn_fd = accept(tcp_socket, (struct sockaddr*)&serv_addr, &len);
            close(tcp_socket);
            bzero(message, sizeof(message));
            bzero(response, sizeof(response));
            recv_tcp(tcp_socket, message);
            if (verbose){
                printf("Message from TCP client:\n%s\n", message);
                puts("----------------------------------------");
            }
            send_tcp(tcp_socket, "ERR\n");
            close(conn_fd);
        }
        // if udp socket is readable receive the message.
        if (FD_ISSET(udp_socket, &rset)) {
            bzero(message, sizeof(message));
            bzero(response, sizeof(response));
            recv_udp(udp_socket, message);
            if (verbose){
                printf("Message from UDP client:\n%s\n", message);
                puts("----------------------------------------");
            }
            if (!parse(udp_socket, message, response))
                send_udp(udp_socket, "ERR\n");
        }
        
    }
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}