#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_MACROS 100
#define MAX_NAME_LEN 63
#define MAX_VALUE_LEN 1023
#define MAX_LINE_LEN 4096

typedef enum {
    TOKEN_EOF,
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_ASSIGN,
    TOKEN_SEMICOLON
} TokenType;

typedef struct {
    TokenType type;
    char text[64];
    int value;
    int line;
    int column;
} Token;

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

typedef struct ASTNode
{
    NodeType type;     // "int"、"+"、"identifier"、"integer"
    int data;             // 整数值或标识符编号
    int pos_x;
    int pos_y;
    struct ASTNode* left_node;
    struct ASTNode* right_node; // [第几行、第几个 token]，方便给出报错信息
} ASTNode;

typedef struct
{
    int str_arr_state;
    int str_arr_pos;
    int str_arr_len;
}key_return;
  //0 表示状态 1表示关键词首地址 2表示关键词长度

typedef struct {
    char name[MAX_NAME_LEN + 1];
    char value[MAX_VALUE_LEN + 1];
} Macro;
static Macro macros[MAX_MACROS];
static int macro_count = 0;

FILE *object_file_w;
FILE *object_file;

// void write_new(FILE* object, char* str)
// {
//     fprintf(object, str);
// }

void str_skip_space()
{

}

void write_new_text(char* key, char* str)
{

}

int str_is_key(char* str, char* key)
{
    return !!strstr(str, key);
}


//关键词检测，返回是否含有这个，有几个单独的key（前后没别的字符），以及他的位置和长度
//并且返回他的位置和长度到传入的数组当中
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
        //key前面的情况
        if (str_key > str && (*(str_key-1) == '_') || isalnum((unsigned char)str[pos_key -1]))
        {
            key_arr[count].str_arr_state = 0;
            count--;
        } //key后面的情况
        if (str_key[key_len] != '\0' && (str_key[key_len] == '_') || isalnum((unsigned char)str[pos_key + key_len]))
        {
            key_arr[count].str_arr_state = 0;
            count--;
        }
        int temp = count++;
        if (count != temp) key_arr[count] = (key_return){1, pos_key, key_len};

        //printf("count: %d\n", count);
        str = str_key + key_len;
        str_key = strstr(str, key);
    }
    return count;
}

//提取两个关键词中的字符串然后存到def_str结构体当中，及其劣质。
static void key_filter(def_str* str_arr,char* text,char* key)
{
    int state = 0;
    char* now_text = text;
    int str_len;
    while (!str_is_key(now_text, key)) now_text++;
    while (isspace((unsigned char)*now_text)) now_text++;
    int i = 0;
    while (*now_text != '\0' && !str_is_key(now_text, key))
    {
        str_arr -> text_key[i++] = *now_text++;
    }
    str_arr->text_key[i] = '\0';
    while (!str_is_key(now_text, key)) now_text++;
    while (isspace((unsigned char)*now_text)) now_text++;
    i = 0;
    while (*now_text != '\0' && !str_is_key(now_text, key))
    {
        str_arr -> text_value[i++] = *now_text++;
    }
    str_arr->text_value[i] = '\0';
}

//遍历文件每一行
static void define_replace(char* str, FILE* object)
{
    int file_line = 0;
    int text_len;
    char *str_temp;
    int count = 0;
    key_return fine_object_key[10];
    def_str object_key;
    while ((str_temp = fgets(str, 1024, object)) != NULL)//遍历每一行
    {
        text_len = strlen(str);
        count = key_is("#define", str_temp, fine_object_key);
        printf("%s %d\n", str_temp, text_len);
        file_line++;
        if (count != 0 && fine_object_key[0].str_arr_pos== 0)
        {
            printf("find #define in line %d\n", file_line);
            key_filter(&object_key, str_temp, " ");
            printf("key: %s, value: %s\n", object_key.text_key, object_key.text_value);
        }

    }

}

int main()
{
    char file_path[256] = {};
    char file_str[1024];
    //printf("plese input path: ");
    //scanf("%s", file_path);
    object_file = fopen("D:\\learn\\minic Compiler\\test.minic", "r");//原本打算设计成输入文件路径
    object_file_w =fopen("D:\\learn\\minic Compiler\\test.minicp", "w");//生成新的编译文件
    define_replace(file_str, object_file);
    fclose(object_file);
    fclose(object_file_w);
    return 0;
}

