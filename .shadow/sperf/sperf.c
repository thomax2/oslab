#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <regex.h>

typedef struct StringNode {
    char *name;
    float time;
    struct StringNode *next;
} StringNode;

StringNode *func_find(char *name_str,StringNode *head)
{
    StringNode *ret = head->next;
    while (ret != NULL)
    {
        if(strcmp(name_str, ret->name) == 0)
            break;
        ret = ret->next;
    }
    return ret;
}

// void insert_node(StringNode *new,StringNode *head)
// {
//     StringNode *end = head;
//     while (end->next != NULL)
//         end = end->next;
//     assert(end->next == NULL);
//     end->next = new;
//     return;    
// }

void free_list(StringNode *head)
{
    StringNode *fore = head;
    StringNode *later = head->next;
    while (later != NULL)
    {
        free(fore->name);
        free(fore);
        fore = later;
        later = fore->next;
    }
    free(fore->name);
    free(fore);
    return;
}

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
        int allTimeNum = 0;
        char regerrbuf[256];
        regex_t reg;
        const char* pattern = "^([^(]+?)\\s*\\(.*\\)\\s*=\\s*[^<]*<([^>]+)>\\s*\n$";
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
        head->next = NULL;

        while (fgets(str,sizeof(str),fd))
        {
            printf("%s\n",str);
            regmatch_t pmatch[3];
            int matchcount = 0;

            char* p = str;
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
                char *NameStr = (char *)malloc((size_t)pmatch[1].rm_eo - (size_t)pmatch[1].rm_so + 1);
                memcpy(NameStr, p + pmatch[1].rm_so,(size_t)pmatch[1].rm_eo - (size_t)pmatch[1].rm_so);
                printf("%s\n",NameStr);
                char *TimeStr = (char *)malloc((size_t)pmatch[2].rm_eo - (size_t)pmatch[2].rm_so + 1);
                memcpy(TimeStr, p + pmatch[2].rm_so,(size_t)pmatch[2].rm_eo - (size_t)pmatch[2].rm_so);
                float oneTime;
                sscanf(TimeStr,"%f",&oneTime);
                free(TimeStr);
                StringNode *findNode = func_find(NameStr,head);
                if(findNode == NULL)
                {
                    StringNode *newNode = (StringNode *)malloc(sizeof(StringNode));
                    newNode->name = NameStr;
                    newNode->time = oneTime;
                    newNode->next = head->next;
                    head->next = newNode;
                    // insert_node(newNode, head);

                }
                else
                    findNode->time += oneTime;

                // continue;
                sumTime += oneTime;
            }
            else
            {
                regerror(c, &reg, regerrbuf, sizeof(regerrbuf));
                regerrbuf[sizeof(regerrbuf) - 1] = '\0';
                printf("%s\n", regerrbuf);
                // break;
                assert(0);
            }

            if(sumTime > 0.1)
            {
                StringNode *iterNode = head->next;
                allTimeNum += 1;
                printf("Time: %f\n",(float)(0.1*allTimeNum));
                while (iterNode != NULL)
                {
                    printf("%s (%f)",iterNode->name,(iterNode->time/sumTime));
                    iterNode->time = 0;
                    iterNode = iterNode->next;
                }
                sumTime = 0;
            }
        }

    }
    // StringNode *iterNode = head->next;
    // allTimeNum += 1;
    // printf("Time: %f\n",(float)(0.1*allTimeNum));
    // while (iterNode != NULL)
    // {
    //     printf("%s (%f)",iterNode->name,(iterNode->time/sumTime));
    //     iterNode->time = 0;
    //     iterNode = iterNode->next;
    // }

    // free_list(head);

}
