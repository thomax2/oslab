#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>


// fork process -> strace target
// pipe strace.stdout > main.stdio
// main parse info


int main(int argc, char *argv[], char *envp[]) {
    // for (int i = 0; i < argc; i++) {
    //     assert(argv[i]);
    //     printf("argv[%d] = %s\n", i, argv[i]);
    // }
    // assert(!argv[argc]);
    char str[200];

    char *exec_argv[] = {"strace","-t",argv[1],NULL};
    // char *exec_envp[] = {}

    // pipe 
    int pipefd[2];
    
    if(pipe(pipefd) == -1)
    {
        perror("pipe wrong\n");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if(pid == 0) //child
    {
        close(pipefd[0]);
        dup2(pipefd[1],STDERR_FILENO);
        execve("/bin/strace", exec_argv, envp);
        // wait(NULL);
    }
    else
    {
        close(pipefd[1]);
        int fd = fopen(pipefd[0],"r");
        while (fgets(str,siezof(str),fd))
        {
            printf("%s\n",str);
        }
    }

}
