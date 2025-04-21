#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    static char line[4096];
    char cfile[] = "/tmp/test/testfile_XXXXXX";

    int tmpfd = mkstemp(cfile);
    int expressNum = 0;

    if(tmpfd == -1)
    {
        perror("mkstemp");
        return 1;
    }

    FILE *fd = fdopen(tmpfd,"w");
    // pid_t pd = fork();
    // fprintf(fd,"good is bad\n");

    while (1) {
        
        printf("crepl> ");
        printf("%s",cfile);
        printf(":");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        char tmpLine[4];
        for (int i = 0; i < 3; i++)
        {
            tmpLine[i] = line[i];
        }
        tmpLine[3] = '\0';

        // func
        if(strcmp(tmpLine,"int") == 0)
        {
            printf("good\n");
            fprintf(fd,"%s\n",line);
            fflush(fd);
        }
        else // express
        {
            printf("bad\n");
            fprintf(fd,"int _expr_wraapper_%d() { return %s; }\n",expressNum,line);
            fflush(fd);
            expressNum++;
        }
        

        // To be implemented.
        printf("Got %zu chars.\n", strlen(line));
    }
}
