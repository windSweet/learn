#include <stdio.h>

typedef int mytype1;
typedef double mytype2;

typedef struct
{
    mytype1 pony_1;
    mytype2 pony_2;

}Pony;




int main()
{
    //指针就是内存地址 pointer
    //指针变量就是内存地址变量 
    int a = 2;
    void* p1 = &a;
    int* p2 = &a;
    *p2 = 1;
    char* p3 = (char*) &a;//强制转换
    *p3 = 3;

    Pony pony_new;
    pony_new.pony_1 = 1;
    printf("%d", pony_new.pony_1);
    return 0;
}