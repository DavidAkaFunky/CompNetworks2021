#include "client.h" 


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
        if (!isdigit(*s++)){
            printf("UID has a non-numeric character. Try again!\n");
            return 0;
        }
    }
    return 1;
}

void create_socket(){   //Creates a socket for the client
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
		exit(EXIT_FAILURE);
    
    memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
    if (getaddrinfo(IP_ADDRESS, PORT, &hints, &res) != 0)
		exit(EXIT_FAILURE);
}

void parse(char* command){
    char name[11]; //The largest command name has 11 characters
    char arg1[SIZE];
    char arg2[SIZE];
    char arg3[SIZE];
    sscanf(command, "%s %s %s %s", name, arg1, arg2, arg3); 
    if (strcmp(arg3, "")){ //Isto deve falhar se tivermos espaços depois do arg2? (Perguntar ao prof)
        puts("Too many arguments. Try again!");
        return;
    }
    if (!strcmp(name, "reg")){
        //Register (UDP): UID (tam 5), pass (tam 8)
        if (!(digits_only(arg1) && has_correct_arg_sizes(arg1, 5, arg2, 8)))
            return;
        reg(IP_ADDRESS,PORT,arg1,arg2);
    } else if (!strcmp(name, "unregister") || !strcmp(name, "unr")){
        //Unegister (UDP): UID (tam 5), pass (tam 8)
        if (!(digits_only(arg1) && has_correct_arg_sizes(arg1, 5, arg2, 8)))
            return;
    } else if (!strcmp(name, "login")){
        //Login (UDP): UID (tam 5), pass (tam 8)
        if (!(digits_only(arg1) && has_correct_arg_sizes(arg1, 5, arg2, 8)))
            return;
    } else if (!strcmp(name, "logout")){
        //Logout (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
    } else if (!strcmp(name, "exit")){
        //Exit (TCP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
    } else if (!strcmp(name, "groups") || !strcmp(name, "gl")){
        //Groups (UDP): (nada)
        if (!has_correct_arg_sizes(arg1, 0, arg2, 0))
            return;
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
    char command[SIZE],ADDRESS[10];
    strcpy(ADDRESS,argv[2]);            //Defines the IP_ADDRESS where the server runs
    sprintf(IP_ADDRESS,"%s%s",ADDRESS,".ist.utl.pt");
    strcpy(PORT,argv[4]);               //Defines the PORT where the server accepts requests
    
    create_socket();                    //NEW

    while(fgets(command, SIZE, stdin)){
        parse(command);
    }
}