#include <stdio.h>

// int a;

int main(int argc, char const *argv[])
{
    int segNum = 7;
    int mod=1;
    int downbound;
    int upbound;
    for(int id=1;id<=5;id++)
    {
        downbound = (segNum + (mod > 0))*(((id-1)<=mod)?(id-1):mod) + (((id-1)>mod)?(id-1-mod):0)*segNum;
        upbound = (segNum + (mod > 0))*(((id)<=mod)?(id):mod) + (((id)>mod)?(id-mod):0)*segNum;
        printf("%d\t%d\n",downbound,upbound);
    }
}
