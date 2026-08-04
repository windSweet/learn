#include <stdio.h>//包含stdio.h这个头文件

/*
char
short
int
long
long long
float
double

%d整数，f是小数，s是字符串
*/

char ch = 'w';
int pony = 10;

int main()//main函数是程序的入口,一个工程中只有一个
{
    //指针就是这个东西在内存中的地址，p指的是pony的地址，而*p是pony变量本身
    //但C语言需要指针，是因为有“改多个变量”和“改大块数据”的需求，返回值解决不了。
    //int *p = &pony;
    //*p = 20;
    //printf("%d\n", *p);
    //printf("hello world\n");
    //printf("%d", 2);
    //printf("%d\n", sizeof(char));

    //int a = 10, b = 20;
    //一条语句定义多个变量
    //变量在使用前一定要赋值
    short a = 10;
    int b = 100;
    long c = 10000;
    long long d = 100000LL;
    //注意long和long的赋值的格式以及打印的时候的格式化操作
    printf("a: %d, b: %d \n", a, b);
    printf("%ld\n", c);
    printf("%lld\n", d);
    return 0;
}