#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <ctype.h>

// 节点类型枚举
typedef enum
{
    NODE_PROGRAM,      // 整个程序（根节点）
    NODE_FUNC_DEF,     // 函数定义
    NODE_NUM,          // 数字常量
    NODE_VAR,          // 变量
    NODE_BINOP,        // 双目运算（+ - * /）
    NODE_ASSIGN,       // 赋值语句 =
    NODE_RETURN,       // return 语句
    NODE_IF,           // if 语句
    NODE_WHILE,        // while 循环
    NODE_BLOCK,        // 代码块 { ... }
    NODE_CALL,         // 函数调用
} NodeType;


//"D:\\learn\\minic Compiler\\test.minic"

typedef struct 
{
    const char *type;     // "int"、"+"、"identifier"、"integer"
    int data;             // 整数值或标识符编号
    int start[2];         // [第几行、第几个 token]，方便给出报错信息
} Token;

typedef struct
{
    int str_arr_state;
    int str_arr_pos;
    int str_arr_len;
}key_return;
  //0 表示状态 1表示关键词首地址 2表示关键词长度

typedef struct
{
    char text_1[100];
    char text_2[100];
}def_str;

static void define_replace(char* str, FILE* object);
static int key_is(const char* key, char* str, key_return key_arr[]);
static void key_filter(def_str* str_arr,char* text, char* key, int text_len);
void write_new(FILE* object, char* str);

FILE *object_file_w;
FILE *object_file;

int main()
{
    char file_path[256] = {};
    char file_str[1024];
    //printf("plese input path: ");
    //scanf("%s", file_path);
    object_file = fopen("D:\\learn\\minic Compiler\\test.minic", "r");//原本打算设计成输入文件路径
    object_file_w =fopen("D:\\learn\\minic Compiler\\test.minicp", "w");//生成新的编译文件
    //define_replace(file_str, object_file);
    char*str_temp1 = fgets(file_str, 1024, object_file);
    //printf("%s", str_temp1);
    key_return object_key[10];
    key_is("#define", str_temp1, object_key);
    if (object_key[0].str_arr_state)
    {
        printf("got");
    }
    else
    {
        printf("nono");
    }
    // def_str str_arr ;
    // key_filter(&str_arr, "#define AB a", " ", strlen("#define AB"));
    // printf("%s %s", str_arr.text_1, str_arr.text_2);

    fclose(object_file);
    fclose(object_file_w);
    return 0;
}

// void write_new(FILE* object, char* str)
// {
//     fprintf(object, str);
// }

void write_new_text(char* key, char* str)
{
    


}

//关键词检测，返回是否含有这个单独的key（前后没别的字符），以及他的位置和长度
int key_is(const char* key, char* str, key_return key_arr[])
{
    int count = 0;
    char* str_key = strstr(str, key);
    if (str_key == NULL)
    {
        return count;
    }
    int pos_key = strstr(str, key) - str;
    int key_len = strlen(key);
    char* str_now;
    while(str_key != NULL)
    {
        pos_key = strstr(str, key) - str;
        if (str_key > str && (*(str_key-1) == '_') || isalnum((unsigned char)str[pos_key -1]))
        {
            key_arr[count].str_arr_state = 0;
            return count;
        } //key前面的情况
        if (str_key[key_len] != '\0' && (str_key[key_len] == '_') || isalnum((unsigned char)str[pos_key + key_len]))
        {
            key_arr[count].str_arr_state = 0;
            return count;
        }
        count++;
        str = str_key + key_len;

    }
    return count;
}

//提取两个关键词中的字符串然后存到def_str结构体当中
//及其劣质。
static void key_filter(def_str* str_arr,char* text, char* key, int text_len)
{
    int state = 0;
    char* now_text = text;
    while (!isspace(*now_text)) now_text++;
    now_text++;
    int i = 0;
    while (*now_text != '\0' && !isspace(*now_text))
    {
        str_arr -> text_1[i++] = *now_text++;
    }
    str_arr->text_1[i] = '\0';
    while (!isspace(*now_text)) now_text++;
    now_text++;
    i = 0;
    while (*now_text != '\0' && !isspace(*now_text))
    {
        str_arr -> text_2[i++] = *now_text++;
    }
    str_arr->text_2[i] = '\0';
}

//遍历文件每一行
static void define_replace(char* str, FILE* object)
{
    int file_line = 0;
    int text_len;
    char *str_temp;
    key_return object_key[10];
    while ((str_temp = fgets(str, 1024, object)) != NULL)//遍历每一行
    {
        text_len = strlen(str);
        printf("%s %d\n", str_temp, text_len);
        file_line++;
        key_is("#define", str_temp, object_key);
        //int char233[file_line];
        if (object_key[0].str_arr_state)
        {
            //object_key = key_is("#define", str_temp);
        }

    }

}