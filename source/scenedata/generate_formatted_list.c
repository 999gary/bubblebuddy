
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    char line[4096];
    int u = 0;
    if (argc >= 3 && !strcmp(argv[2], "-u")) u = 1;
    while (fgets(line, sizeof(line), stdin)) {
        char *c = line + strlen(line) - 1;
        while (isspace(*c)) *c-- = 0;
        if (u) {
            for (char *c = line; *c; c++) *c = toupper(*c);
        }
        
        //printf(argv[1], line[0], line[1], line[2], line[3], line[0], line[1], line[2], line[3]);
        printf(argv[1], line);
        printf("\n");
    }
}
