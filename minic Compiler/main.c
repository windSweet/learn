#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//"D:\\learn\\minic Compiler\\test.minic"

typedef struct 
{
    const char *type;     // "int"、"+"、"identifier"、"integer"
    int data;             // 整数值或标识符编号
    int start[2];         // [第几行、第几个 token]，方便给出报错信息
} Token;

typedef struct 
{
   int str_arr[3];
}key_return;
  //0 表示状态 1表示关键词首地址 2表示关键词长度
  

static void define_replace(char* str, FILE* object);
static key_return key_is(const char* key, char* str);

int main()
{
    char file_path[256] = {};
    char file_str[1024];
    //printf("plese input path: ");
    //scanf("%s", file_path);
    FILE *object_file = fopen("D:\\learn\\minic Compiler\\test.minic", "r");//原本打算设计成输入文件路径
    //define_replace(file_str, object_file);
    char*str_temp1 = fgets(file_str, 1024, object_file);
    printf("%s", str_temp1);
    if (key_is("#define", str_temp1).str_arr[0])
    {
        printf("got");
    }
    else
    {
        printf("nono");
    }
    

    return 0;
}

//关键词检测
key_return key_is(const char* key, char* str)
{
    key_return key_arr;
    char* str_key = strstr(str, key);
    if (str_key != NULL)
    {
        int pos_key = strstr(str, key) - str;
        int key_len = strlen(key);
        if (str_key > str && (*(str_key-1) == '_') || isalnum((unsigned char)str[pos_key -1]))
        {
            key_arr.str_arr[0] = 0;
            return key_arr;
        } //key前面的情况
        if (str_key[key_len] != '\0' && (str_key[key_len] == '_') || isalnum((unsigned char)str[pos_key + key_len]))
        {
            key_arr.str_arr[0] = 0;
            return key_arr;
        }
        key_arr.str_arr[0] = 1;
        key_arr.str_arr[1] = pos_key;
        key_arr.str_arr[2] = key_len;
        return key_arr;
    }
    else
    {
        key_arr.str_arr[0] = 0;
        return key_arr;
    }

}

static void key_filter()
{

}

//遍历文件每一行
static void define_replace(char* str, FILE* object)
{
    int file_line = 0;
    int text_len;
    char *str_temp;
    key_return object_key;
    while ((str_temp = fgets(str, 1024, object)) != NULL)//遍历每一行
    {
        text_len = strlen(str);
        printf("%s %d\n", str_temp, text_len);
        file_line++;
        //int char233[file_line];
        if (key_is("#define", str_temp).str_arr[0])
        {
            object_key = key_is("#define", str_temp);
        }
        

        // for (int i = 0; i < strlen(str); i++)
        // {
        //     // if ((strstr(str_temp, "#define") != NULL))
        //     // {
        //     //     printf("%d", str_temp - strstr(str_temp, "#define"));
        //     //     break;
        //     // }
        // }
        
    }


}