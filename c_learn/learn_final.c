//malloc memory allocation 申请连续空间
//alloc contiguous allocation 申请空间+数据初始化
//realloc re-allocation 修改空间大小
//free 释放空间
#include <stdio.h>
#include <stdlib.h>

//c语言内存结构分为
//代码区：代码运行时加载到这里
//栈；函数调用的时候进栈执行，如果函数的数据前加static就会变成全局变量存储到
//静态区（分为初始化和未初始化）
//常量区：就比如指针定义的字符串
//堆就是malloc，calloc，realloc这些申请的内存


int main()
{
    char *file_path = "file_learn\\purpose.c";// 这个符号\就是转义 \\s 打印出来就是 \s
    printf("%s", file_path );
    int *p = malloc(10 * sizeof(int));
    //malloc 返回的是void指针但是c语言这时候自动强转了
    //malloc 如果申请内存多会先用虚拟内存（等真正需要复制的时候再使用），然后如果还申请会造成内存泄漏返回null
    //malloc 返回的是首地址，所以尽量先定义总大小
    //malloc 申请的内存没有初始化，需要先赋值才可以使用

    //realloc 相比 malloc多了一条置0的初始化
    for (int i = 0; i < 10; i++)
    {
        p[i] = (i + 1) * 10;
        //p[i] = *(p + i), i[p] = *(p + i)
    }
    for (int i = 0; i < 10; i++)
    {
        printf("%d  ", p[i]);
    }
    
    //realloc申请新内存的时候如果后面占用就会重现申请一块地址，但是原本的数据不会改变，但是首地址会改变，并且原来的内存会free掉
    int *pp = realloc(p, 20 * sizeof(int));
    if (pp == NULL) printf("no");
    else
    {
        printf("no no\n");
    }
    free(pp);
    free(p);

}
