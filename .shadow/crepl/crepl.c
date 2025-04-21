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
            fprintf(fd,"%s\n",line);
        }
        else // express
        {
            fprintf(fd,"int _expr_wraapper_%d() { return %s; }",expressNum,line);
            expressNum++;
        }
        

        // To be implemented.
        printf("Got %zu chars.\n", strlen(line));
    }
}
