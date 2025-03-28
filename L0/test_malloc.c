#include <am.h>
#include <klib.h>


int main(void)
{
    // printf("%p\t%p\n",heap.start,heap.end);
     
    int *a = (int*)malloc(sizeof(int));
    char *b = (char*)malloc(sizeof(char)*20);
    *a = 10;
    strcpy(b,"hello malloc");
    printf("%d\t%s\n",*a,b);
    return 0;
}
