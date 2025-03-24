#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static char *__itoa(int num, char *buff, int base)
{
    static const char sym[] = "0123456789abcdef";
    bool is_neg = false;

    if(num == 0)
    {
        strcpy(buff,"0");
        return buff;
    }
    else if (num < 0)
    {
        is_neg = true;
        strcpy(buff, "-");
        buff++;
        num = -num;
    }

    int i = 1;
    while((int)(num/(i*base)) != 0)
        i *= base;


    int j = 0;
    for (; i != 0; j++)
    {
        // printf("%d\n",num / base);
        buff[j] = sym[num / i];
        num = num % i;
        i /= base;
    }
    buff[j] = '\0';
    return is_neg ? (buff - 1) : buff;
}

static char *__ptoa(void *p, char *buff)
{
    static const char sym[] = "0123456789abcdef";

    long num = (long)p;

    buff[0] = '0';
    buff++;
    buff[0] = 'x';
    buff++;
    long i = 1;
    while ((long)(num / (i * 16)) != 0)
        i *= 16;

    int j = 0;
    for (; i != 0; j++)
    {
        buff[j] = sym[num / i];
        num = num % i;
        i /= 16;
    }

    buff[j] = '\0';
    return buff-2;

}

int main()
{
   int num = 123456;
	char buff[64];
	__ptoa(buff,buff);
	printf("%s\n",buff);
   return 0;
}