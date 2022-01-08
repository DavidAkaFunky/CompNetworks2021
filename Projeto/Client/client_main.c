#include "client.h"
#include "../common.h"
#include <sys/types.h>
#include <sys/socket.h>

/**
 * @brief Activates timeout limit (given by the constant TIME) for client sockets.
 * 
 * @param[in] sd the socket whose timer will be activated.
 * @return 0 if the activation using setsockopt was successful, -1 otherwise.
 */
int timer_on(int sd){
    struct timeval tmout;
    memset((char *)&tmout,0,sizeof(tmout)); /* Clear time structure. */
    tmout.tv_sec = TIME; /* Wait for TIME seconds (check client.h) for a reply from server. */
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
 * @brief Checks if there's a user currently logged in.
 * 
 * @param uid the string that contains the currently logged in user's ID, if existent.
 * @param log flag used to display a message to the user.
 * @return true if there's a user currently logged in.
 * @return false otherwise.
 */
bool check_login(char *uid, bool log){
    if (strlen(uid) != 5){
        if (log)
            puts(NO_LOGIN);
        return false;
    }
    return true;
}

/**
 * @brief Checks if there's a group currently selected.
 * 
 * @param gid the string that contains the currently selected group's ID, if existent.
 * @return true if there's a group currently selected.
 * @return false otherwise.
 */
bool check_group(char *gid){
    if (strlen(gid) != 2){
        puts(NO_GROUP);
        return false;
    }
    return true;
}

/**
 * @brief Create a socket object.
 * 
 * @param res information about the address of the service provider.
 * @param socktype the socket type (SOCK_DGRAM for UDP, SOCK_STREAM for TCP)
 * @param ip_address the IP address where the packets will be sent.
 * @param port the port used to send the packets to the desired IP address.
 * @return the file descriptor of the created socket. 
 */
int create_socket(struct addrinfo **res, int socktype, char* ip_address, char* port){
    int sockfd = socket(AF_INET,socktype,0);
    if (sockfd == -1){
        puts(SOCK_FAIL);
        exit(EXIT_FAILURE);
    }
    struct addrinfo hints;
    bzero(&hints, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = socktype;
    if(getaddrinfo(ip_address, port, &hints, res) != 0){
        puts(ADDR_FAIL);
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

// Esta função provavelmente é inútil - Perguntar aos profs!!!
bool is_valid_IP(char* ip_address){
    char part1[20], part2[4], part3[4], part4[4];
    bzero(part1, 4);
    bzero(part2, 4);
    bzero(part3, 4);
    bzero(part4, 4);
    if (sscanf(ip_address, "%[^.].%[^.].%[^.].%[^.]", part1, part2, part3, part4) == 4 &&
        0 < atoi(part1) && atoi(part1) < 255 && 0 < atoi(part2) && atoi(part2) < 255 &&
        0 < atoi(part3) && atoi(part3) < 255 && 0 < atoi(part4) && atoi(part4) < 255)
            return strlen(part1) <= 3 && strlen(part2) <= 3 && strlen(part3) <= 3 && strlen(part4) <= 3;
            
    return true;
}

/**
 * @brief Get the local IP address.
 * 
 * @param ip_address the IP address where the packets will be sent.
 * @return true if the function could get the local IP address.
 * @return false otherwise.
 */
bool get_local_IP(char* ip_address){
    char hostbuffer[256];
    
    if (gethostname(hostbuffer, sizeof(hostbuffer)) == -1)
        return false;

    struct hostent *host_entry = gethostbyname(hostbuffer);
    if (host_entry == NULL)
        return false;
    
    strcpy(ip_address, inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])));
    return strlen(ip_address) > 0;
}

/**
 * @brief Parse the argv to get the IP address and port needed to communicate with the server.
 * 
 * @param ip_address the IP address where the packets will be sent.
 * @param port the port used to send the packets to the desired IP address.
 * @param argc 
 * @param argv 
 * @return true if the argv is well-formatted.
 * @return false otherwise.
 */
bool parse_argv(char* ip_address, char* port, int argc, char** argv){
    if (!(argc == 1 || argc == 3 || argc == 5) || strcmp(argv[0], "./user"))
        return false;
    bzero(ip_address, 512);
    bzero(port, 6);
    if (argc >= 3){
        if (!strcmp(argv[1], "-n")){
            /*if (!is_valid_IP())
                return false;*/
            strcpy(ip_address, argv[2]); // Copy the IP address given in the argv
            if (argc > 3){
                if (!strcmp(argv[3], "-p") && digits_only(argv[4], "port number")){ // Check if there's a valid port number
                    strcpy(port, argv[4]); // Copy the port given in the argv
                    return true;
                }
                return false;
            }
            strcpy(port, "58026"); // Use port 58000 + group number (26) if there's no specified port number
            return true;
        }
        if (!strcmp(argv[1], "-p") && digits_only(argv[2], "port number")){
            strcpy(port, argv[2]);
            if (argc > 3){
                if (!strcmp(argv[3], "-n") /* && is_valid_IP();*/){ // Check if there's a valid IP address
                    strcpy(ip_address, argv[4]); // Copy the IP address given in the argv
                    return true; 
                }
                return false;
            }
            strcpy(ip_address, "localhost"); // Get local IP address if there's no specified address
            return true; 
        }
    }
    
    if (argc == 1){
        strcpy(port, "58026"); // Use port 58000 + group number (26) if there's no specified port number
        strcpy(ip_address, "localhost"); // Get local IP address if there's no specified address
            return true;
    }

    return false;
}

/**
 * @brief Parse each line of the stdin and call a command if the line is well-formatted.
 * 
 * @param udp_socket the udp socket used to communicate
 * @param res information about the address of the service provider.
 * @param ip_address the IP address where the packets will be sent.
 * @param port the port used to send the packets to the desired IP address.
 * @param command the string that will receive the command.
 * @param uid the currently logged user's ID, if existent.
 * @param password the currently logged user's password, if there's a user.
 * @param gid the currently selected group's ID, if existent.
 */
void parse(int udp_socket, struct addrinfo *res, char* ip_address, char* port, char* command, char* uid, char* password, char* gid){
    char name[12]; // The largest command name has 11 characters + '\0'
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    bzero(name, 12);
    bzero(arg1, SIZE);
    bzero(arg2, SIZE);
    bzero(arg3, SIZE);
    if (sscanf(command, "%s ", name) < 1){ // Check if there's at least a command name
        puts(INVALID_CMD);
        return;
    }
    command += strlen(name) + 1; // The name has already been read!
    if (!strcmp(name, "post")){
        // Post (TCP): arg1 = text, arg2 = FName (optional)
        // There must be a logged in user and a group selected
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
        if (!(check_login(uid, true) && check_group(gid)))
            return;
        post(ip_address, port, gid, uid, res, arg1, arg2);
        return;
    }
    sscanf(command, "%s %s %[^\n]", arg1, arg2, arg3);
    if (strcmp(arg3, "")){ // Isto deve falhar se tivermos espaços depois do arg2? (Perguntar ao prof)
        puts("Too many arguments. Please try again!");
        return;
    }
    if (!strcmp(name, "reg")){
        // Register (UDP): arg1 = uid (size 5, digits), arg2 = pass (size 8, alphanumerical)
        reg(arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "unregister") || !strcmp(name, "unr")){
        // Unegister (UDP): arg1 = uid (size 5, digits), arg2 = pass (size 8, alphanumerical)
        unreg(arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "login")){
        // Login (UDP): uid (size 5, digits), pass (size 8, alphanumerical)
        // There can't be a user already logged in
        if (!check_login(uid, false)){ 
            if (login(arg1, arg2, res, udp_socket) == 1){
                strcpy(uid, arg1);
                strcpy(password, arg2);
            }
            return;
        }
        puts(LOGIN_DOUBLE);
    } else if (!strcmp(name, "logout")){
        // Logout (UDP): (none)
        // There must be a user logged in
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true)))
            return;
        if (logout(uid, password, res, udp_socket) == 1)
            bzero(uid, 6);
    } else if (!strcmp(name, "showuid") || !strcmp(name, "su")){
        // Show UID : (none)
        // There must be a user logged in
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true)))
            return;
        printf("The uid selected is %s.\n",uid);
    } else if (!strcmp(name, "exit")){
        // Exit (TCP): (none)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        freeaddrinfo(res);
        if(check_login(uid, false))
            logout(uid, password, res, udp_socket);
        close(udp_socket);
        exit(EXIT_SUCCESS);
    } else if (!strcmp(name, "groups") || !strcmp(name, "gl")){
        // Groups (UDP): (none)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        groups(res, udp_socket);
    } else if (!strcmp(name, "subscribe") || !strcmp(name, "s")){
        // Subscribe (UDP): gid (size 2, digits), group_name (size 24, alphanumerical)
        if (!check_login(uid, true))
            return;
        subscribe(uid, arg1, arg2, res, udp_socket);
    } else if (!strcmp(name, "unsubscribe") || !strcmp(name, "u")){
        // Unsubscribe (UDP): gid (size 2, digits)
        if (!digits_only(arg1, "gid"))
            return;
        add_trailing_zeros(atoi(arg1), 2, arg1);
        if (!has_correct_arg_sizes(arg1, 2, arg2, 0))
            return;
        unsubscribe(uid, arg1, res, udp_socket);
    } else if (!strcmp(name, "my_groups") || !strcmp(name, "mgl")){
        // My groups (UDP): (none)
        // There must be a user logged in
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true)))
            return;
        my_groups(uid, res, udp_socket);
    } else if (!strcmp(name, "select") || !strcmp(name, "sag")){
        // Select: gid (size 2, digits)
        if (!digits_only(arg1, "gid"))
            return;
        add_trailing_zeros(atoi(arg1), 2, arg1);
        if (!(has_correct_arg_sizes(arg1, 2, arg2, 0) && check_login(uid, true)))
            return;
        if (!strcmp(arg1, "00")){
            puts(GRP_ZERO);
            return;
        }
        strcpy(gid, arg1);
        printf("Group %s sucessfully selected.\n", gid);
    } else if (!strcmp(name, "showgid") || !strcmp(name, "sg")){
        // Show GID : (none)
        // There must be a logged in user and a group selected
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true) && check_group(gid)))
            return;
        printf("The group selected is %s.\n", gid);
    } else if (!strcmp(name, "ulist") || !strcmp(name, "ul")){
        // User list (TCP): (none)
        // There must be a logged in user and a group selected
        if (!(has_correct_arg_sizes(arg1, 0, arg2, 0) && check_login(uid, true) && check_group(gid)))
            return;
        ulist(ip_address, port, gid, res);
    } else if (!strcmp(name, "retrieve") || !strcmp(name, "r")){
        // Retrieve (TCP): arg1 = MID (size 4, digits)
        // There must be a logged in user and a group selected
        if (!digits_only(arg1, "message ID"))
            return;
        add_trailing_zeros(atoi(arg1), 4, arg1);
        if (!(has_correct_arg_sizes(arg1, 4, arg2, 0) && check_login(uid, true) && check_group(gid)))
            return;
        retrieve(ip_address, port, gid, uid, arg1, res);
    } else
        puts(INVALID_CMD);
}


int main(int argc, char** argv){
    char command[SIZE], uid[6], password[9], gid[3], ip_address[512], port[6];
    if (!parse_argv(ip_address, port, argc, argv)){
        puts(ARGV_ERR);
        exit(EXIT_FAILURE);
    }
    struct addrinfo *res;
    int udp_socket = create_socket(&res, SOCK_DGRAM, ip_address, port);
    bzero(uid, 6);
    bzero(password, 9);
    bzero(gid, 3);

    // Create a folder for the files that are downloaded using the retrieve command
    if (mkdir("DOWNLOADS", 0700) == -1 && access("DOWNLOADS", F_OK)){
        puts(DOWNLOADS_FAIL);
        exit(EXIT_FAILURE);
    }

    // Read commands
    while(fgets(command, SIZE, stdin)){
        parse(udp_socket, res, ip_address, port, command, uid, password, gid);
        bzero(command, SIZE);
        puts("----------------------------------------");
    }

    close(udp_socket);
    freeaddrinfo(res);
    exit(EXIT_SUCCESS);
}