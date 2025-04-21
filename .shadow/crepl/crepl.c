#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    static char line[4096];
    // char cfile[] = "/tmp/test/testfile_XXXXXX";

    // int tmpfd = mkstemp(cfile);
    int expressNum = 0;

    // if(tmpfd == -1)
    // {
    //     perror("mkstemp");
    //     return 1;
    // }

    FILE *fd = fopen("/tmp/crepl/env.c","w");
    
    // pid_t pd = fork();
    // fprintf(fd,"good is bad\n");

    while (1) {
        
        printf("crepl> ");
        // printf("%s",cfile);
        printf(":");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        size_t len = strlen(line);
        if(len>0 && line[len-1] == '\n')
            line[len-1] = '\0';


        char tmpLine[4];
        for (int i = 0; i < 3; i++)
        {
            tmpLine[i] = line[i];
        }
        tmpLine[3] = '\0';

        FILE *fdtmp = fopen("/tmp/crepl/tmp.c","w");

        // func
        if(strcmp(tmpLine,"int") == 0)
        {
            fprintf(fdtmp,"%s\n",line);
            fflush(fdtmp);
        }
        else // express
        {
            fprintf(fdtmp,"int _expr_wraapper_%d() { return %s; }\n",expressNum,line);
            fflush(fdtmp);
            expressNum++;
        }
        pid_t pd = fork();
        if (pd == 0)
        {
            const char *target_dir = "/tmp/crepl";
    
            if (chdir(target_dir) != 0) {
                perror("chdir failed");
                return EXIT_FAILURE;
            }
            freopen("/dev/null","w",stdout);
            int ret = execl("/bin/sh","sh","-c","make tmp",(char *)NULL);
        }
        else
        {
            wait();

        }
        

        // To be implemented.
        // printf("Got %zu chars.\n", strlen(line));
    }
}
