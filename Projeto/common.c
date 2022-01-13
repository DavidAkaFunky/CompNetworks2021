#include "common.h"

/**
 * @brief Verifies if it has only has digits.
 * 
 * @param s the string that is being verified.
 * @param name the name of the parameter that is being verified.
 * @return the value that indicates success or failure.
 */
int digits_only(char *s, char* name){
    while (*s) {
        if (!isdigit(*s)){
            if (name != NULL)
                printf("The %s has a non-numeric character. Please try again!\n", name);
            return 0;
        }
        s++;
    }
    return 1;
}

/**
 * @brief Verifies if it is alphanumeric.
 * 
 * @param s the string that is being verified.
 * @param flag the flag that defines the specification of the verification.
 * @param log flag that makes it verbose.
 * @return the value that indicates success or failure.
 */
int is_alphanumerical(char* s, int flag, bool log){
    while (*s) {
        if (!(isalpha(*s) || isdigit(*s))){
            switch (flag){
                case 0:
                    if(!(*s == 32)){
                        if (log)
                            puts(NO_ALPH0);
                        return 0;
                    }
                    break;
                case 1:
                    if(!(*s == 45 || *s == 95)){
                        if (log)
                            puts(NO_ALPH1);
                        return 0;
                    }
                    break;
                case 2:
                    if(!(*s == 45 || *s == 46 || *s == 95)){
                        if (log)
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

/**
 * @brief Verifies if it has the correct size.
 * 
 * @param arg the parameter that is being verified.
 * @param name the name of the parameter that is being verified.
 * @param size the correct size of the parameter.
 * @return the value that indicates success or failure.
 */
int is_correct_arg_size(char* arg, char* name, int size){
    if (strlen(arg) != size){
        if (name != NULL)
            printf("The %s's size is %ld, instead of %d. Please try again!\n", name, strlen(arg), size);
        return 0;
    }
    return 1;
}

/**
 * @brief 
 * 
 * @param arg1 the first parameter that is being verified.
 * @param name1 the name of the first parameter that is being verified.
 * @param size1 the correct size of the first parameter.
 * @param arg2 the second parameter that is being verified.
 * @param name2 the name of the second parameter that is being verified.
 * @param size2 the correct size of the second parameter.
 * @return the value that indicates success or failure.
 */
int has_correct_arg_sizes(char* arg1, char* name1, int size1, char* arg2, char* name2, int size2){
    return is_correct_arg_size(arg1, name1, size1) && is_correct_arg_size(arg2, name2, size2);
}

/**
 * @brief Updates the string given with the number of zeros needed.
 * 
 * @param number the original number that will have zeros added.
 * @param len length of the string desired.
 * @param result the result string after adding zeros.
 */
void add_trailing_zeros(int number, int len, char* result){
    memset(result, '0', len);
    int i = 1;
    while (number > 0){
        result[len-i] = '0' + number % 10;
        ++i;
        number /= 10;
    }
}

