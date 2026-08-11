#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

int function_1(int num_1, int num_2)
{
    int sum = num_1 + num_2;
    printf("Hello World\n");
    return sum;
}

double function_2(double num_1, double num_2);

void arr_printer(int arr[], int len)
{
    //int len = s izeof(arr)/sizeof(arr[0]);
    //注意 数组只有在定义处才是完整的数组，而在作为参数传入函数当中时，他只是传入一个变量，相当于一个指针(8byte)，即首地址
    //数组的本质就是好多块变量，通过偏移形成索引，即[0]就是偏移0
    for (int i = 0; i <= len-1; i++)
    {
        printf("%d\n", arr[i]);
    }
    return;
}



int main()
{
    int arr_1[] = {33, 22, 32, 99, 100};
    printf("hello world\n");
    // 取余最后的正负和第一个数字有关系
    // 隐式转换1. short char类型的数据在运算时候会先升为int再进行运算
    // 2. char<short<int<long<long long<float<double
    // 强制转换就是去掉多余的byte，所以会导致数据丢失
    //int b = 243;
    //short i = (short)b;
    int i = 10;
    int j = 5;
    int p = i++ + ++i - --j - i--;    //这种写法真的太傻逼了，还要看编译器的设置，项目不会遇到的
    printf("%d\n", p);
    i = 100;
    printf(("%d\n"), i);


    //这里的逻辑是后缀是先取值后自增，前缀是先自增后取值
    //有的编译是这样的逻辑不是从左往右，而是先前缀后后缀（并且后缀是在表达式中变量都用完后再进行自增
    //例如本题他就会先 先2i是11然后3j是4，然后再看1i和4i都是11，然后同时自增

    //运算符和python差不多，唯一有区别的是多了个三元运算符 例如 a>b ? a, b
    int c = i > j ? i : j;//意思是如果前面表达式成立就输出a值，不成立输出b值
    printf("%d\n",c);

    int a = 11;
    a = 'a';
    /*
    if (a > 10)
    {
        printf("the first solution");
    }
    else if(a <= 10)
    {
        printf("the second solution");
    }
    */
    switch (a)
    {
    case 'a':case 1://这里不支持变量
        printf("the first solution\n");
        break;
    
    default:
        printf("the other solutions\n");
        break;
    }

    int num_1;
    int num_2;
    for(num_1 = 1; num_1 < 10; num_1++)
    {
        for (num_2 = 1; num_2 < num_1+1; num_2++)
        {
            printf("%d * %d = %d", num_2, num_1, num_1*num_2);
        }
        printf("\n");
    }
    // /t是补齐八个空格
    long long res = time(NULL);
    printf("%lld\n", res);
    /*
    //猜数字
    srand(time(NULL));
    int real_answer = rand() % 100 + 1;
    int answer;
    printf("%d\n", real_answer);
    while (1)
    {
        printf("what's your answer:");
        scanf("%d", &answer);
        if (answer == real_answer){printf("you are right\n"); break;}
        else if (answer > real_answer){printf("bigger\n");}
        else if (answer < real_answer){printf("smaller\n");}
        
    }
    */
    arr_printer(arr_1, 5);
    


    return 0;
}

double function_2(double num_1, double num_2)
{
    double sum = num_1 * num_2;
    return sum;
}