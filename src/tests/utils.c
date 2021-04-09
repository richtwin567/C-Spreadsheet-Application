#include "../utils/utils.h"

#include <stdlib.h>

int main(int argc, char const *argv[])
{
    char *str = malloc(15);

    strcpy(str, "     trim left");
    printf("[%s]\n", str);
    ltrim(str);
    printf("[%s]\n", str);

    strcpy(str, "trim right    ");
    printf("[%s]\n", str);
    rtrim(str);
    printf("[%s]\n", str);

    strcpy(str, "     trim     ");
    printf("[%s]\n", str);
    trim(str);
    printf("[%s]\n", str);

    return 0;
}
