#include <stdio.h>

/*
char
short
int
long
long long
float
double

%d
*/

char ch = 'w';
int pony = 10;

int main()//main主函数，首先执行这个，程序中有且只有一个
{
    
    int a, b;
    printf("你好");
    scanf("%d %d", &a, &b);//将两个整数类型存入到a和b，输入的值要高度保持一致
    printf("a = %d, b = %d\n", a, b);
    return 0;
}