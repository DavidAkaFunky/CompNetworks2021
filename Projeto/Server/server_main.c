#include "server.h"
#include "../common.h"

struct sockaddr_in client_addr;
socklen_t addrlen;

int udp_receive(int udp_socket, char* message){
    addrlen = sizeof(client_addr);
    ssize_t nread = recvfrom(udp_socket, message, 128, 0, (struct sockaddr*)&client_addr, &addrlen);
    if (nread == -1){
        puts(RECV_ERR);
        return -1;
    }
    return nread;
}

int udp_send(int udp_socket, char* message, bool verbose){
    if (verbose)
        printf("Message sent: %s", message);
    addrlen = sizeof(client_addr);
    ssize_t n = sendto(udp_socket, message, strlen(message), 0, (struct sockaddr*)&client_addr,addrlen);
    if (n == -1){
        puts(SEND_ERR);
        return -1;
    }
    return n;
}

int socket_bind(int socktype, char* port, struct addrinfo** res){
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
    
    if (getaddrinfo(NULL, port, &hints, res) != 0){
        puts(ADDR_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    if (bind(sockfd,(*res)->ai_addr,(*res)->ai_addrlen) == -1){
        puts(BIND_FAIL);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}


int parse_argv(int argc, char** argv, char* port, bool* verbose){
    if (argc < 1 || argc > 4 || strcmp(argv[0], "./DS"))
        return 0;
    bzero(port, 6);
    if (argc >= 2){
        if (!strcmp(argv[1], "-v")){
            *verbose = true;
            if (argc > 2){
                if (!strcmp(argv[2], "-p") && digits_only(argv[3], "port number")){
                    strcpy(port, argv[3]);
                    return 1;
                }
                return 0;
            }
            strcpy(port, "58026");
            return 1;
        }
        if (!strcmp(argv[1], "-p") && argc > 2 && digits_only(argv[2], "port number")){
            strcpy(port, argv[2]);
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
        strcpy(port, "58026");
        return 1;
    }
    return 0;
}

void show_client_info(char* protocol, char* message){
    char client[NI_MAXHOST];
    getnameinfo((struct sockaddr *) &client_addr, addrlen, client, sizeof(client), NULL, 0, 0);
    printf("Command from %s client: %s\nSent by: %s\nIP address: %s\nPort: %d\n", protocol, message, client, inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
}

bool parse_udp(int udp_socket, char* message, bool verbose){
    char name[4], arg1[SIZE], arg2[SIZE], arg3[SIZE], arg4[SIZE];
    char delim1, delim2, delim3, delim4;
    bzero(name, 4);
    bzero(arg1, SIZE);
    bzero(arg2, SIZE);
    bzero(arg3, SIZE);
    sscanf(message, "%[^ \n]", name);
    if (verbose)
        show_client_info("UDP", name);
    message += strlen(name);
    if (!strcmp(name, "REG")){        
        //Register
        sscanf(message, "%c%[^ \n]%c%[^ \n]%c", &delim1, arg1, &delim2, arg2, &delim3);
        return delim1 == ' ' && delim2 == ' ' && delim3 == '\n' && reg(udp_socket, arg1, arg2, verbose);
    } else if (!strcmp(name, "UNR")){
        //Unegister
        sscanf(message, "%c%[^ \n]%c%[^ \n]%c", &delim1, arg1, &delim2, arg2, &delim3);
        return delim1 == ' ' && delim2 == ' ' && delim3 == '\n' && unreg(udp_socket, arg1, arg2, verbose);
    } else if (!strcmp(name, "LOG")){
        //Login
        sscanf(message, "%c%[^ \n]%c%[^ \n]%c", &delim1, arg1, &delim2, arg2, &delim3);
        return delim1 == ' ' && delim2 == ' ' && delim3 == '\n' && login(udp_socket, arg1, arg2, verbose);
    } else if (!strcmp(name, "OUT")){
        //Logout
        sscanf(message, "%c%[^ \n]%c%[^ \n]%c", &delim1, arg1, &delim2, arg2, &delim3);
        return delim1 == ' ' && delim2 == ' ' && delim3 == '\n' && logout(udp_socket, arg1, arg2, verbose);
    } else if (!strcmp(name, "GLS")){
        //Groups
        sscanf(message, "%c", &delim1);
        return delim1 == '\n' && groups(udp_socket, verbose);
    } else if (!strcmp(name, "GSR")){
        //Subscribe
        sscanf(message, "%c%[^ \n]%c%[^ \n]%c%[^ \n]%c", &delim1, arg1, &delim2, arg2, &delim3, arg3, &delim4);
        return delim1 == ' ' && delim2 == ' ' && delim3 == ' ' && delim4 == '\n' && subscribe(udp_socket, arg1, arg2, arg3, verbose);
    } else if (!strcmp(name, "GUR")){
        //Unsubscribe
        sscanf(message, "%c%[^ \n]%c%[^ \n]%c", &delim1, arg1, &delim2, arg2, &delim3);
        return delim1 == ' ' && delim2 == ' ' && delim3 == '\n' && unsubscribe(udp_socket, arg1, arg2, verbose);
    } else if (!strcmp(name, "GLM")){
        //My groups
        sscanf(message, "%c%[^ \n]%c%[^ \n]%c", &delim1, arg1, &delim2);
        return delim1 == ' ' && delim2 == '\n' && my_groups(udp_socket, arg1, verbose);
    }
    return false;
}

bool parse_tcp(int conn_fd, char* message, bool verbose){
    if (!strcmp(message, "ULS ")){
        //User list (TCP)
        return ulist(conn_fd, verbose);
    } else if (!strcmp(message, "PST ")){
        //Post (TCP)
        return post(conn_fd, verbose);
    } else if (!strcmp(message, "RTV ")){
        //Retrieve (TCP):
        return retrieve(conn_fd, verbose);
    }
    return false;
}

int main(int argc, char** argv){
    bool verbose = false;
    char port[6];
    if (!parse_argv(argc, argv, port, &verbose)){
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
    
    // Create e bind server's UDP and TCP sockets
    struct addrinfo *res;
    int udp_socket = socket_bind(SOCK_DGRAM, port, &res);
    int tcp_socket = socket_bind(SOCK_STREAM, port, &res);
    listen(tcp_socket, 10);
    char message[BUF_SIZE];
    fd_set rset;
    int conn_fd;

    // Clear the descriptor set
    FD_ZERO(&rset);
 
    // Get maxfd
    int maxfd = udp_socket > tcp_socket ? udp_socket + 1 : tcp_socket + 1;
    while (true) {
        // Set tcp_socket and udp_socket in readset
        FD_SET(tcp_socket, &rset);
        FD_SET(udp_socket, &rset);
 
        // Select the ready descriptor
        int nready = select(maxfd, &rset, NULL, NULL, NULL);
 
        // If tcp socket is readable then handle it by accepting the connection
        if (FD_ISSET(tcp_socket, &rset)) {
            socklen_t len = sizeof(client_addr);
            conn_fd = accept(tcp_socket, (struct sockaddr*)&client_addr, &len);
            bzero(message, sizeof(message));
            tcp_read(conn_fd, message, 4);
            if (verbose)
                show_client_info("TCP", message);
            if (!parse_tcp(conn_fd, message, verbose))
                tcp_send(conn_fd, "ERR\n",4);
            if (verbose)
                puts("----------------------------------------");
            close(conn_fd);
        }
        // If udp socket is readable receive the message.
        if (FD_ISSET(udp_socket, &rset)) {
            bzero(message, sizeof(message));
            udp_receive(udp_socket, message);
            if (!parse_udp(udp_socket, message, verbose))
                udp_send(udp_socket, "ERR\n", verbose);
            if (verbose)
                puts("----------------------------------------");
        }
    }
    close(tcp_socket);
    close(udp_socket);
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}