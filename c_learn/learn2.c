#include <stdio.h>

typedef int mytype1;
typedef double mytype2;

typedef struct
{
    mytype1 pony_1;
    mytype2 pony_2;

}Pony;

int arr[] = {12, 234 ,24, 657, 34, 76};
int len = sizeof(arr)/sizeof(arr[0]);

//二分查找。数据必须是有序的（倒序的时候可以交换max和min）
int find_void(int arr[], int len, int num)
{
    int min = 0;
    int max = len - 1;//下标从零开始
    while (min <= max)
    {
        int mid = (min + max)/2;
        if (arr[mid] < num)
        {
            min = mid;
        }
        else if (arr[mid] > num)
        {
            max = mid;
        }
        else if (arr[mid] = num)
        {
            return mid;
        }
    }
    return -1;
}

//用指针是因为，函数传参的时候是传递值不是传递内存中的变量本身
void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;

}

int *method()
{
    static int a = 10;
    int *p = &a;
    return p;
}
//函数有生命周期，函数中的变量会在函数结束后被重新刷新掉，
//如果前面加static就会让这个变量在程序结束时候再被刷新

//野指针指向未分配的空间
//悬空指针指的是指针指向的内存已经被释放了，例如前面method函数如果没加static就会让a释放
//这两种指针都要避免否则会影响其他程序的运行

//void指针随便赋值地址,但是无法获取里面的数据，也不可以加减

void swap_void(void *a, void *b, int len);
int main()
{
    int a = 10;
    int b = 11;
    swap_void(&a, &b, 4);
    printf("%d, %d", a, b);

    //int* p = method();
    //printf("%d\n", *p);


    /*
    int a = 12;
    int b = 123;
    swap(&a, &b);
    printf("%d, %d", a, b);

    */
    
    //int a = find_void(arr, len, 657);
    //printf("%d", a);

    /*
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
    */

    return 0;
}

//可以看出来void函数就是可以让函数可以处理任何形式的数据类型
//下面这个就是表示交换任何类型的两个变量的值
void swap_void(void *a, void *b, int len)
{
    char *p1 = (char*)a;//主要是因为char类型只占一个字节
    char *p2 = (char*)b;
    char temp = 0;
    //交换变量的每个字节
    for (int i = 0; i < len; i++)
    {
        temp = p1[i];
        p1[i] = p2[i];
        p2[i] = temp;


        //p1++;//这里就是后移一个字节
        //p2++;
    }
}