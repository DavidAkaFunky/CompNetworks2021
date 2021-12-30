#include "server.h"

ssize_t bytes;

void reg(char* uid, char* pass, char* response){
    char path[12];
    sprintf(path,"USERS/%s",uid);

    if (!(digits_only(uid,"UID") && has_correct_arg_sizes(uid, 5, pass, 8) && is_alphanumerical(pass, 0))){
        sprintf(response,"RRG NOK\n");
        return;
    }
    if (!access(path, F_OK)){//if already exists the uid directory
        sprintf(response,"RRG DUP\n");
        return;
    }
    //create a folder with the user uid, and a txt file with his password
    FILE *fp;
    char filepath[26];
    sprintf(filepath,"%s/%s_pass.txt",path,uid);
    mkdir(path);
    fp = fopen (filepath, "w");
    fprintf(fp,pass);               //nao sei se é preciso que o ficheiro tenha um /n no fim mas acho que nao
    fclose (fp);
    
    sprintf(response,"RRG OK\n");
}