#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <regex.h>

typedef struct StringNode {
    char *str;
    float time;
    struct StringNode *next;
} StringNode;



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

    char *exec_argv[] = {"strace","-T",argv[1],NULL};
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
        float sumTime = 0.0;
        char regerrbuf[256];
        regex_t reg;
        const char* pattern = "^(.*?)\\s*\\([^)]*\\)\\s*=\\s*[^<]*<([^>]+)>$";
        const size_t matchsz = 3;

        // file way open pipe port
        close(pipefd[1]);
        FILE *fd = fdopen(pipefd[0],"r");

        // 
        int c = regcomp(&reg, pattern, REG_EXTENDED);
        if (0 != c)
        {
            perror("reg error\n");
            exit(1);
        }
        StringNode *head = (StringNode *)malloc(sizeof(StringNode));
        assert(head!=NULL);


        while (fgets(str,sizeof(str),fd))
        {
            // printf("%s\n",str);
            regmatch_t pmatch[3];
            int matchcount = 0;

            const char* p = str;
            c = regexec(&reg, p, matchsz, pmatch, 0);
            if (REG_NOMATCH == c)
            {
                /** 没有找到匹配结束循环 */
                printf("MATCH FINISHED\n");
                regfree(&reg);
                // break;
            }
            else if (0 == c)
            {
                /** 找到匹配,则输出匹配到的所有捕获组(catch group) */
                // printf("%d MATCH (%d-%d)\n", ++matchcount, pmatch[0].rm_so, pmatch[0].rm_eo);
                // for (int i = 1; i < matchsz; ++i)
                // {
                //     printf("group %d :<<", i);
                //     print_str(p, pmatch[i].rm_so, pmatch[i].rm_eo);
                //     printf(">>\n");
                // }
                char *NameStr = (char *)malloc((size_t)pmatch[1].rm_eo - (size_t)pmatch[1].rm_so + 1);
                memcpy(NameStr, p + pmatch[1].rm_so,(size_t)pmatch[1].rm_eo - (size_t)pmatch[1].rm_so);
                printf("%s\n",NameStr);
                continue;
            }
            else
            {
                regerror(c, &reg, regerrbuf, sizeof(regerrbuf));
                regerrbuf[sizeof(regerrbuf) - 1] = '\0';
                printf("%s\n", regerrbuf);
                // break;
                assert(1);
            }

        }
    }

}
