#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>


// fork process -> strace target
// pipe strace.stdout > main.stdio
// main parse info


int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
        assert(argv[i]);
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);

    

    // pipe 
    // int pipefd[2];
    
    // if(pipe(pipefd) == -1)
    // {
    //     perror("pipe wrong\n");
    //     exit(EXIT_FAILURE);
    // }

    // pid_t pid = fork();

    // if(pid == 0) //child
    // {
    //     close(pipefd[0]);
    // }
    // else
    // {
    //     close(pipefd[1]);
    // }

}
