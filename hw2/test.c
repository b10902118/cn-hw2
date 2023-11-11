#include <stdio.h>

#include <string.h>
int main(int argc, char **argv) {
    char s[] = "ababcdef";
    char *p = strtok(s, "ab");
    printf("p: '%s'\n", p);

    return 0;
}
