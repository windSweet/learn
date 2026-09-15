#include <stdio.h>

int main()
{
    int x;
    int i;
    int num;
    int state;
    int j;
    
    x = 10;
    i = 0;
    num = 1;
    state = 1;
    j = 2;

    while (i < x)
    {
        state = 1;
        num = num + 1;
        j = 2;

        while(j*j <= num)
        {
            if (num % j == 0)
            {
                state = 0;
            }
            j = j + 1;
        }
        if(state)
        {
            i = i + 1;
        }
    }
    printf("%d", num);
    return 0;
}