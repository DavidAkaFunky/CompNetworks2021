#include <stdio.h>
#include <string.h>

int main(){
    char x[10], y[200], z[10];
    int ret;
    if ((ret = scanf("%s \"%[^\"]\" %[^\n]", x, y, z)) != 1)
        printf("ERROR %d\n", ret);
    puts(x);
    puts(y);
    puts(z);
    if ((ret = scanf("\"%[^\"]\" %s", y, z)) != 1)
        printf("ERROR %d\n", ret);
    puts(x);
    puts(y);
    puts(z);
    return 0;
}