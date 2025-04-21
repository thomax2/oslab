#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    static char line[4096];
    
    pid_t pd = fork();

    while (1) {
        
        printf("crepl> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        // To be implemented.
        printf("Got %zu chars.\n", strlen(line));
    }
}
