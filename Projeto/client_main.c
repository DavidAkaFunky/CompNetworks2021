#include "client.h" 

char IP_ADDRESS[20], PORT[20];
int errcode;
struct addrinfo hints, *res;

int is_alphanumerical(char* s, int flag){
    while (*s) {
        if (!(isalpha(*s) || isdigit(*s))){
            if (flag){
                if(!(*s == 45 || *s == 95)){
                    puts("The argument doesn't contain only alphanumerical characters, - or _. Try again!");
                    return 0;
                }
                s++;
                continue;
            }
            puts("The argument doesn't contain only alphanumerical characters. Try again!");
            return 0;
        }
        s++;
    }
    return 1;
}

int is_correct_arg_size(char* arg, int size){
    if (strlen(arg) != size){
        printf("%s's size is not %d. Try again!\n", arg, size);
        return 0;
    }
    return 1;
}

int has_correct_arg_sizes(char* arg1, int size1, char* arg2, int size2){
    return is_correct_arg_size(arg1, size1) && is_correct_arg_size(arg2, size2);
}

int digits_only(char *s){
    while (*s) {
        if (!isdigit(*s)){
            puts("UID has a non-numeric character. Try again!\n");
            return 0;
        }
        s++;
    }
    return 1;
}

int create_socket(){   //Creates a socket for the client
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1){
        puts("Failed creating the socket!");
        exit(EXIT_FAILURE);
    }
    
    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(IP_ADDRESS, PORT, &hints, &res) != 0){
        puts("Failed getting the address' information!");
        exit(EXIT_FAILURE);
    }
	return fd;
}

void parse(int fd, char* command, char* uid, char* password){
    char name[12]; //The largest command name has 11 characters '\0'
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    memset(name, 0, 12);
    memset(arg1, 0, SIZE);
    memset(arg2, 0, SIZE);
    memset(arg3, 0, SIZE);
    sscanf(command, "%s %s %s %s", name, arg1, arg2, arg3);
    if (strcmp(arg3, "")){ //Isto deve falhar se tivermos espaços depois do arg2? (Perguntar ao prof)
        puts("Too many arguments. Try again!");
        return;
    }
    if (!strcmp(name, "reg")){
        //Register (UDP): UID (tam 5), pass (tam 8)
        if (!(digits_only(arg1) && has_correct_arg_sizes(arg1, 5, arg2, 8) && is_alphanumerical(arg2, 0)))
            return;
        reg(IP_ADDRESS, arg1, arg2, res, fd);
    } else if (!strcmp(name, "unregister") || !strcmp(name, "unr")){
        //Unegister (UDP): UID (tam 5), pass (tam 8)
        if (!(digits_only(arg1) && has_correct_arg_sizes(arg1, 5, arg2, 8) && is_alphanumerical(arg2, 0)))
            return;
        unreg(IP_ADDRESS, arg1, arg2, res, fd);
    } else if (!strcmp(name, "login")){
        //Login (UDP): UID (tam 5), pass (tam 8)
        if (!(digits_only(arg1) && has_correct_arg_sizes(arg1, 5, arg2, 8) && is_alphanumerical(arg2, 0)))
            return;
        if (login(IP_ADDRESS, arg1, arg2, res, fd) == 1){
            strcpy(uid, arg1);
            strcpy(password, arg2);
        } 
    } else if (!strcmp(name, "logout")){
        //Logout (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        if (logout(IP_ADDRESS, uid, password, res, fd) == 1)
            memset(uid, 0, 6);
    } else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
    } else if (!strcmp(name, "groups") || !strcmp(name, "gl")){
        //Groups (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
        groups(IP_ADDRESS, res, fd);
    } else if (!strcmp(name, "subscribe") || !strcmp(name, "s")){
        //Subscribe (UDP): GID (tam 2), GName (tam 24)
        if (!has_correct_arg_sizes(arg1, 2, arg2, 24))
            return;
    } else if (!strcmp(name, "unsubscribe") || !strcmp(name, "u")){
        //Unsubscribe (UDP): GID (tam 2)
        if (!has_correct_arg_sizes(arg1, 2, arg2, 24))
            return;
    } else if (!strcmp(name, "my_groups") || !strcmp(name, "mgl")){
        //My groups (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
    } else if (!strcmp(name, "select") || !strcmp(name, "sag")){
        //Select (UDP): GID (tam 2)
        if (!has_correct_arg_sizes(arg1, 2, arg2, 0))
            return;
    } else if (!strcmp(name, "ulist") || !strcmp(name, "ul")){
        //User list (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
    } else if (!strcmp(name, "post")){
        //Post (TCP): "text" (Verificar as aspas, talvez?), [FName] (Verificar os parênteses, talvez?)
    } else if (!strcmp(name, "retrieve") || !strcmp(name, "r")){
        //Retrieve (TCP): MID
        if (!has_correct_arg_sizes(arg1, 0/*???*/, arg2, 0))
            return;
    } else
        puts(INVALID_CMD);
}

int main(int argc, char* argv[]){

    char command[SIZE], ADDRESS[10], uid[6], password[9];
    strcpy(IP_ADDRESS,argv[2]);            //Defines the IP_ADDRESS where the server runs
    //sprintf(IP_ADDRESS,"%s%s",ADDRESS,".ist.utl.pt");
    strcpy(PORT,argv[4]);               //Defines the PORT where the server accepts requests
    int fd = create_socket();
    memset(uid, 0, 6);
    memset(password, 0, 9);
    while(fgets(command, SIZE, stdin)){
        parse(fd, command, uid, password);
        memset(command, 0, SIZE);
    }
}