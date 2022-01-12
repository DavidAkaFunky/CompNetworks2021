#include "common.h"

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

void add_trailing_zeros(int number, int len, char* result){
    memset(result, '0', len);
    int i = 1;
    while (number > 0){
        result[len-i] = '0' + number % 10;
        ++i;
        number /= 10;
    }
}

