#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    static char line[4096];
    char cfile[] = "/tmp/test/testfile_XXXXXX";

    int fd = mkstemp(cfile);

    if(fd == -1)
    {
        perror("mkstemp");
        return 1;
    }

    // pid_t pd = fork();


    while (1) {
        
        printf("crepl> ");
        printf("%s",cfile);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        // To be implemented.
        printf("Got %zu chars.\n", strlen(line));
    }
}
