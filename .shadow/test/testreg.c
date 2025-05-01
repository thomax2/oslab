
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

void print_str(const char* input, size_t _start, size_t _end)
{
	if (input)
	{
		for (size_t i = _start; i < _end; ++i)
		{
			printf("%c", input[i]);
		}
	}
}


int main(viod) {

    const char* inputstr = "execve(\"/usr/bin/ls\", [\"ls\"], 0x7ffdecd8fb88 /* 63 vars */) = 0 <0.000424>\n";

    char regerrbuf[256];
	regex_t reg;

    const char* pattern = "^(.*?)\\s*\\([^)]*\\)\\s*=\\s*[^<]*<([^>]+)>\r\n$";

    int c = regcomp(&reg, pattern, REG_EXTENDED);
    if (0 != c)
    {
        perror("reg error\n");
        exit(1);
    }

    const size_t matchsz = 3;
	regmatch_t pmatch[3];
	/** 起始匹配的偏移量 */
	size_t offset = 0;
	/** 捕获计数         */
	int matchcount = 0;

    while(1)
    {
        const char* p = inputstr + offset;
        c = regexec(&reg, p, matchsz, pmatch, 0);
        if (REG_NOMATCH == c)
        {
            /** 没有找到匹配结束循环 */
            printf("MATCH FINISHED\n");
            break;
        }
        else if (0 == c)
		{
			/** 找到匹配,则输出匹配到的所有捕获组(catch group) */
			printf("%d MATCH (%d-%d)\n", ++matchcount, pmatch[0].rm_so, pmatch[0].rm_eo);
			for (int i = 0; i < matchsz; ++i)
			{
				printf("group %d :<<", i);
				print_str(p, pmatch[i].rm_so, pmatch[i].rm_eo);
				printf(">>\n");
			}
			offset += pmatch[0].rm_eo;
			continue;
		}
		else
		{
			/************************************************************************/
			/** regexec 调用出错输出错误信息,结束循环                               */
			/************************************************************************/
			regerror(c, &reg, regerrbuf, sizeof(regerrbuf));
			regerrbuf[sizeof(regerrbuf) - 1] = '\0';
			printf("%s\n", regerrbuf);
			break;
		}
    }
    printf("%d MATCH FOUND\n", matchcount);
    regfree(&reg);
	return 0;

}