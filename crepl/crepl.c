#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dlfcn.h>

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

    while (1) {
        printf("crepl> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        // get line last byte is '\n', replace to '\0'
        size_t len = strlen(line);
        if(len>0 && line[len-1] == '\n')
            line[len-1] = '\0';


        // get first three byte to compare "int"
        char tmpLine[4];
        for (int i = 0; i < 3; i++)
            tmpLine[i] = line[i];
        tmpLine[3] = '\0';

        // tmp.c use to judge syntax validity
        // store tmp.c first, if validity copy to env.c
        FILE *fdtmp = fopen("/tmp/crepl/tmp.c","w");

        // func
        if(strcmp(tmpLine,"int") == 0)
        {
            fprintf(fdtmp,"%s\n",line);
            fflush(fdtmp);
        }
        else // express, use wrapper function to wrap express
        {
            fprintf(fdtmp,"int _expr_wrapper_%d() { return (%s); }\n",expressNum,line);
            fflush(fdtmp);
        }

        // child process to judge validity and copy to env.c
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
        else  // wait child process over
            wait(NULL);
        
        
        // if not function, excute this command 
        if(strcmp(tmpLine,"int") != 0){
            
            // child process to generate so file
            pid_t pd = fork();
            if(pd == 0)
            {
                const char *target_dir = "/tmp/crepl";
    
                if (chdir(target_dir) != 0) {
                    perror("chdir failed");
                    return EXIT_FAILURE;
                }
                freopen("/dev/null","w",stdout);
                freopen("/dev/null","w",stderr);
                int ret = execl("/bin/sh","sh","-c","make env",(char *)NULL);    
            }
            else
                wait(NULL);


            // open so file and excute function
            void *handle;
            int (*expr)(void);
            char *error;

            handle = dlopen("/tmp/crepl/crepl.so", RTLD_LAZY);
            if (!handle) {
                fprintf(stderr, "%s\n", dlerror());
                return 1;
            }
            dlerror();

            // get expr function name
            char exprName[30];
            sprintf(exprName,"_expr_wrapper_%d",expressNum);

            *(int **) (&expr) = dlsym(handle, exprName);

            if ((error = dlerror()) != NULL)  {
                fprintf(stderr, "%s\n", error);
                dlclose(handle);
                return 1;
            }
            printf("%d\n",expr());
            dlclose(handle);
            expressNum++;
        }
        fclose(fdtmp);
    }
    fclose(fd);
}
