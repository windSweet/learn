/*
#include <stdio.h>


char
short
int
long1.1
long long
float1.2
double
%d
char ch = 'w';
int pony = 10;

int main()//main主函数，首先执行这个，程序中有且只有一个
{
    
    double a, b, c;
    printf("请输入长宽高\n");
    scanf("%lf %lf %lf", &a, &b, &c);
    printf("a的面积是%.2lf\n", a*b);
    printf("b的面积是%.2lf\n", a*c);
    printf("c的面积是%.2lf\n", b*c);
    
    double volument;
    printf("长方体的体积%.2f", a*b*c);
    

}
*/

#include <stdio.h>
#include <string.h>

//本质上字符串就是个数组，然后他会在最后自动补上一个\0表示结束，所以如果数组这个括号里需要加数字就是字符数+1
char a[100] = "hello world";
//可以修改字符串

char *b = "abcd";
char *c = "abcd";
//不可以修改,只读常量，但是可以复用，如果在定义一个相同内容的变量，他不会再创建一个（复用机制）


char arr[5][100] = 
{
    "yuanshen",
    "wangzhe",
    "huoying",
    "zhou",
    "chiji"
};

//

int main()
{
    char str[100];
    printf("please input some words: ");
    scanf("%s", str);
    int num;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] <= 'z' && str[i] >= 'a')//ASCII实现的
        {
            num++;
        }
        
    }
    printf("%d", num);



    // printf("%d\n", strlen(a));
    // strcat(a, b);//拼接，前提是a中有足够的空间，并且a是可以修改的
    // //strcopy拷贝
    // //strcmp是比较两个字符串，相同的话返回0，不相同返回非零
    // //strupr,strlwr是全变大写或者小写
    // printf("%s\n",a);


    // a[0] = 'H';
    // printf("%s\n", a);

    // printf("%p\n", b);
    // printf("%p\n", c);//输出结果一样

    // for (int i = 0; i < 5; i ++)
    // {
    //     char *p1 = arr[i];
    //     printf("%s\n", p1);//这里不用解引用是因为%s会自己解引用
    // }
}