#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <regex.h>
#include <time.h>
#include <fcntl.h>
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
    char str[2000];

    // char *exec_argv[] = {"strace","-T",argv[1],NULL};
    char **exec_argv = malloc(sizeof(char *)*(argc+2));
    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    exec_argv[argc+1] = NULL;
    for (size_t i = 0; i < argc-1; i++)
        exec_argv[i+2] = argv[i+1];

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
        int fd = open("/dev/null", O_WRONLY);
        dup2(fd, STDOUT_FILENO);
        dup2(pipefd[1],STDERR_FILENO);
        execve("/bin/strace", exec_argv, envp);
        // wait(NULL);
    }
    else
    {
        float sumTime = 0.0;
        int allTimeNum = 0;
        int refFlag = 0;
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
        time_t start_time = time(NULL);
        time_t end_time = time(NULL);
        while (fgets(str,sizeof(str),fd))
        {
            if(refFlag == 1)
            {
                start_time = time(NULL);
                refFlag = 0;
            }

            // printf("%s\n",str);
            regmatch_t pmatch[3];
            int matchcount = 0;

            char* p = str;
            c = regexec(&reg, p, matchsz, pmatch, 0);
            if (REG_NOMATCH == c)
            {
                /** 没有找到匹配结束循环 */
                // printf("MATCH FINISHED\n");
                // regfree(&reg);
                continue;
            }
            else if (0 == c)
            {
                /** 找到匹配,则输出匹配到的所有捕获组(catch group) */
                char *NameStr = (char *)malloc((size_t)pmatch[1].rm_eo - (size_t)pmatch[1].rm_so + 1);
                memcpy(NameStr, p + pmatch[1].rm_so,(size_t)pmatch[1].rm_eo - (size_t)pmatch[1].rm_so);
                // printf("%s\n",NameStr);
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

            end_time = time(NULL);
            if(difftime(end_time, start_time) > 0.1)
            {
                printf("====================\n");
                StringNode *iterNode = head->next;
                allTimeNum += 1;
                printf("Time: %f\n",(float)(0.1*allTimeNum));
                float maxTime[5]={0.0};
                char *maxName[5];
                while (iterNode != NULL)
                {
                    // printf("%s (%f%%)",iterNode->name,(iterNode->time/sumTime)*(100));
                    // iterNode->time = 0;
                    // iterNode = iterNode->next;
                    for (int i = 0; i < 5; i++)
                    {
                        if(iterNode->time > maxTime[i])
                        {
                            maxTime[i] = iterNode->time;
                            maxName[i] = iterNode->name;
                            break;;
                        }
                    }
                    iterNode->time = 0;
                    iterNode = iterNode->next;
                }
                for (int i = 0; i < 5; i++)
                {
                    printf("%s (%f%%)\n",maxName[i],(maxTime[i]/sumTime)*(100));
                }
                
                sumTime = 0;
                refFlag = 1;
            }
        }

        StringNode *iterNode = head->next;
        printf("====================\n");
        printf("Time: %f\n",(float)sumTime);
        float maxTime[5]={0.0};
        char *maxName[5];
        while (iterNode != NULL)
        {
            // if((iterNode->time/sumTime)*(100) > 0.1){
            //     printf("%s (%f%%)\n",iterNode->name,(iterNode->time/sumTime)*(100));
            //     iterNode->time = 0;
            //     iterNode = iterNode->next;    
            // }
            for (int i = 0; i < 5; i++)
            {
                if(iterNode->time > maxTime[i])
                {
                    maxTime[i] = iterNode->time;
                    maxName[i] = iterNode->name;
                    break;
                }
            }
            iterNode->time = 0;
            iterNode = iterNode->next;
        }
        for (int i = 0; i < 5; i++)
        {
            printf("%s (%f%%)\n",maxName[i],(maxTime[i]/sumTime)*(100));
        }
        free_list(head);
    }
    return 0;
}
