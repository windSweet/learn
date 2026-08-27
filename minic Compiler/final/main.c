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
    /* 关键字 */
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    /* 字面量与标识符 */
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    /* 算术运算符 */
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    /* 赋值和比较 */
    TOKEN_ASSIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    /* 分隔符 */
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_COMMA,
    TOKEN_SEMICOLON
} TokenType;


//lexer状态
typedef struct {
    const char *source;
    size_t pos;
    int line;
    int column;
} Lexer;

typedef struct {
    TokenType type;
    char text[64];
    int length;
    long value;
    int line;//所在行
    int column;//所在列
} Token;

typedef struct ASTNode ASTNode;

typedef struct ASTNodeList {
    ASTNode *node;
    struct ASTNodeList *next;
} ASTNodeList;

typedef enum
{
    AST_PROGRAM, //根节点
    AST_FUNCTION,// 函数定义
    AST_BLOCK,// 代码块 { ... }

    AST_VAR_DECL,
    AST_ASSIGN,// 赋值语句 =
    AST_IF,// if 语句
    AST_WHILE,// while 循环
    AST_RETURN,// return 语句
    AST_EXPR_STMT,

    AST_BINARY,// 双目运算（+ - * /）
    AST_VARIABLE,// 变量
    AST_NUMBER,// 数字常量
    AST_CALL// 函数调用

} ASTNodeType;

struct ASTNode {
    ASTNodeType type;
    Token token;

    union {
        struct {
            ASTNodeList *functions;
        } program;

        struct {
            char name[64];
            ASTNodeList *parameters;
            ASTNode *body;
        } function;

        struct {
            ASTNodeList *statements;
        } block;

        struct {
            char name[64];
            ASTNode *initializer;
        } variable_declaration;

        struct {
            ASTNode *target;
            ASTNode *value;
        } assignment;

        struct {
            ASTNode *condition;
            ASTNode *then_branch;
            ASTNode *else_branch;
        } if_statement;

        struct {
            ASTNode *condition;
            ASTNode *body;
        } while_statement;

        struct {
            ASTNode *value;
        } return_statement;

        struct {
            TokenType operator;
            ASTNode *left;
            ASTNode *right;
        } binary;

        struct {
            long value;
        } number;

        struct {
            char name[64];
        } variable;

        struct {
            char name[64];
            ASTNodeList *arguments;
        } call;

        struct {
            ASTNode *expression;
        } expression_statement;
    } as;
};


typedef struct {
    char name[MAX_NAME_LEN + 1];
    char value[MAX_VALUE_LEN + 1];
} Macro;

static Macro macros[MAX_MACROS];
static int macro_count = 0;

//确认是字母或者_开头
static int is_identifier_start(char ch)
{
    return isalpha((unsigned char)ch) || ch == '_';
}
static int is_identifier_char(char ch)
{
    return isalnum((unsigned char)ch) || ch == '_';
}


//查找这个宏
static int find_macro(const char *name)
{
    int i;

    for (i = 0; i < macro_count; i++)
    {
        if (strcmp(macros[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

//添加宏，存在就添加替换，不存在就添加
static void add_macro(const char *name, const char *value)
{
    int index = find_macro(name);

    if (index >= 0) {
        strcpy(macros[index].value, value);
        return;
    }

    if (macro_count >= MAX_MACROS) {
        fprintf(stderr, "错误：宏数量超过上限\n");
        exit(EXIT_FAILURE);
    }

    strcpy(macros[macro_count].name, name);
    strcpy(macros[macro_count].value, value);
    macro_count++;
}

//创建token
static Token make_token(TokenType type, int line, int column)
{
    Token token = {0};

    token.type = type;
    token.line = line;
    token.column = column;
    token.value = 0;
    token.text[0] = '\0';

    return token;
}

//lexer的报错
static void lexer_error(const Lexer *lexer, const char *message)
{
    fprintf(stderr, "词法错误：第 %d 行，第 %d 列：%s\n",
        lexer->line, lexer->column, message
    );
    exit(EXIT_FAILURE);
}

//lexer的初始化
static void lexer_init(Lexer *lexer, const char *source)
{
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 1;
}

//保存当前字符
static char lexer_current(const Lexer *lexer)
{
    return lexer->source[lexer->pos];
}

//查看下一个字符
static char lexer_peek(const Lexer *lexer)
{
    if (lexer->source[lexer->pos] == '\0')
    {
        return '\0';
    }

    return lexer->source[lexer->pos + 1];
}

//实现对lexer的更新
static void lexer_advance(Lexer *lexer)
{
    char current = lexer->source[lexer->pos];

    if (current == '\0') {
        return;
    }

    lexer->pos++;

    if (current == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
}

//创建单字符token
static Token lexer_read_single_char_token(Lexer *lexer, TokenType type)
{
    Token token = make_token(
        type,
        lexer->line,
        lexer->column
    );

    token.text[0] = lexer_current(lexer);
    token.text[1] = '\0';

    lexer_advance(lexer);

    return token;
}

//创建双字符token
static Token lexer_read_double_char_token(Lexer *lexer, TokenType type)
{
    Token token = make_token(
        type,
        lexer->line,
        lexer->column
    );

    token.text[0] = lexer_current(lexer);
    lexer_advance(lexer);

    token.text[1] = lexer_current(lexer);
    lexer_advance(lexer);

    token.text[2] = '\0';

    return token;
}

//创建数字token
static Token lexer_read_number(Lexer *lexer)
{
    Token token = make_token(TOKEN_NUMBER, lexer->line, lexer->column);
    token.type = TOKEN_NUMBER;
    token.value = 0;
    token.line = lexer->line;
    token.column = lexer->column;
    while (isdigit((unsigned char)lexer->source[lexer->pos])) {
        token.value =
            token.value * 10 +
            (lexer->source[lexer->pos] - '0');

        lexer_advance(lexer);
    }

    return token;
}

//创建标识符token
static Token lexer_read_identifier(Lexer *lexer)
{
    Token token = make_token(TOKEN_NUMBER, lexer->line, lexer->column);
    size_t length = 0;

    token.line = lexer->line;
    token.column = lexer->column;

    while (
        isalnum((unsigned char)lexer_current(lexer)) ||
        lexer_current(lexer) == '_'
    ) {
        if (length < sizeof(token.text) - 1) {
            token.text[length++] = lexer_current(lexer);
        }

        lexer_advance(lexer);
    }

    token.text[length] = '\0';

    if (strcmp(token.text, "int") == 0) token.type = TOKEN_INT;
    else if (strcmp(token.text, "return") == 0) token.type = TOKEN_RETURN;
    else if (strcmp(token.text, "if") == 0) token.type = TOKEN_IF;
    else if (strcmp(token.text, "else") == 0) token.type = TOKEN_ELSE;
    else if (strcmp(token.text, "while") == 0) token.type = TOKEN_WHILE;
    else token.type = TOKEN_IDENTIFIER;

    return token;
}

//跳过空格
static void lexer_skip_whitespace(Lexer *lexer)
{
    while (isspace((unsigned char)lexer_current(lexer))) {
        lexer_advance(lexer);
    }
}

//lexer总函数
static Token lexer_next_token(Lexer* lexer)
{
    lexer_skip_whitespace(lexer);
    char current = lexer_current(lexer);
    if (current == '\0')
    {
        return make_token(TOKEN_EOF, lexer->line, lexer->column);
    }

    //识别关键字或标识符，第一个字符只能是字母或下划线
    if (is_identifier_start(current))
    {
        return lexer_read_identifier(lexer);
    }
    //识别数字
    if (isdigit((unsigned char)current))
    {
        return lexer_read_number(lexer);
    }

    //识别运算符和分隔符
    switch (current)
    {
    case '+':
        return lexer_read_single_char_token(lexer, TOKEN_PLUS);
        break;
    case '-':
        return lexer_read_single_char_token(lexer, TOKEN_MINUS);
        break;
    case '*':
        return lexer_read_single_char_token(lexer, TOKEN_STAR);
        break;
    case '/':
        return lexer_read_single_char_token(lexer, TOKEN_SLASH);
        break;
    case '%':
        return lexer_read_single_char_token(lexer, TOKEN_PERCENT);
        break;
    case '=':
        if (lexer_peek(lexer) == '=')
        {
            return lexer_read_double_char_token(lexer, TOKEN_EQUAL);
        }
        return lexer_read_single_char_token(lexer, TOKEN_ASSIGN);
    case '!':
        if (lexer_peek(lexer) == '=')
        {
            return lexer_read_double_char_token(lexer, TOKEN_NOT_EQUAL);
        }
        lexer_error(lexer, "ehh, sorry the ! can't dependly appear");
    case '<':
        if (lexer_peek(lexer) == '=')
        {
            return lexer_read_double_char_token(lexer, TOKEN_LESS_EQUAL);
        }
        return lexer_read_single_char_token(lexer, TOKEN_LESS);
        break;
    case '>':
        if (lexer_peek(lexer) == '=')
        {
            return lexer_read_double_char_token(lexer, TOKEN_GREATER_EQUAL);
        }
        return lexer_read_single_char_token(lexer, TOKEN_GREATER);
        break;
    case '(':
        return lexer_read_single_char_token(lexer, TOKEN_LPAREN);
        break;
    case ')':
        return lexer_read_single_char_token(lexer, TOKEN_RPAREN);
        break;
    case '{':
        return lexer_read_single_char_token(lexer, TOKEN_LBRACE);
        break;
    case '}':
        return lexer_read_single_char_token(lexer, TOKEN_RBRACE);
        break;
    case ',':
        return lexer_read_single_char_token(lexer, TOKEN_COMMA);
        break;
    case ';':
        return lexer_read_single_char_token(lexer, TOKEN_SEMICOLON);
        break;
    default:
        break;
    }

}


//parser部分

//创建node函数
static ASTNode* ast_new(ASTNodeType type, Token token)
{
    ASTNode* node = calloc(1, sizeof(ASTNode));
    if (node == NULL)
    {
        printf("have no enough memory");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    node->token = token;
    return node;
}

//创建num node
static ASTNode *ast_new_number(Token token)
{
    ASTNode *node = ast_new(AST_NUMBER, token);
    node->as.number.value = token.value;
    return node;
}

//创建变量节点
static ASTNode *ast_new_variable(Token name_token)
{
    ASTNode *node = ast_new(AST_VARIABLE, name_token);
    strcpy(node->as.variable.name, name_token.text);
    return node;
}

//创建二元表达式node
static ASTNode *ast_new_binary(Token operator_token, ASTNode *left, ASTNode *right)
{
    ASTNode *node = ast_new(AST_BINARY, operator_token);
    node->as.binary.operator = operator_token.type;
    node->as.binary.left = left;
    node->as.binary.right = right;
    return node;
}

//创建变量声明节点 没有初始化表达式时：initializer == NULL
static ASTNode *ast_new_var_decl(Token int_token, Token name_token, ASTNode *initializer)
{
    ASTNode *node = ast_new(AST_VAR_DECL, int_token);

    //防止栈溢出
    snprintf(node->as.variable_declaration.name,
        sizeof(node->as.variable_declaration.name),"%s",
        name_token.text);
    node->as.variable_declaration.initializer = initializer;
    return node;
}

//创建赋值节点
static ASTNode *ast_new_assignment(Token assign_token, ASTNode *target, ASTNode *value)
{
    ASTNode *node = ast_new(AST_ASSIGN, assign_token);
    //as.assignment.target原本是name,但是为了防止变量交换所以改成了这样
    node->as.assignment.target = target;
    node->as.assignment.value = value;
    return node;
}

//创建if节点
/*
AST_IF
├── condition
│   └── AST_BINARY <
│       ├── AST_VARIABLE a
│       └── AST_NUMBER 1
├── then
│   └── AST_BLOCK
│       └── AST_RETURN
└── else
    └── NULL
*/
static ASTNode *ast_new_if(Token if_token, ASTNode *condition,
    ASTNode *then_branch, ASTNode *else_branch)
{
    ASTNode *node = ast_new(AST_IF, if_token);
    node->as.if_statement.condition = condition;
    node->as.if_statement.then_branch = then_branch;
    node->as.if_statement.else_branch = else_branch;
    return node;
}

//创建while节点
static ASTNode *ast_new_while(Token while_token, ASTNode *condition, ASTNode *body)
{
    ASTNode *node = ast_new(AST_WHILE, while_token);
    node->as.while_statement.condition = condition;
    node->as.while_statement.body = body;
    return node;
}

//创建代码块节点
static ASTNode *ast_new_block(Token open_brace)
{
    ASTNode *node = ast_new(AST_BLOCK, open_brace);
    node->as.block.statements = NULL;
    return node;
}
//向代码块添加语句
static void ast_block_add_statement(ASTNode *block, ASTNode *statement)
{
    if (block == NULL || block->type != AST_BLOCK)
    {
        fprintf(stderr, "error, not block\n");
        exit(EXIT_FAILURE);
    }
    ast_list_append(&block->as.block.statements, statement);
}



//创建return节点
static ASTNode *ast_new_return(Token return_token, ASTNode *value)
{
    ASTNode *node = ast_new(AST_RETURN, return_token);
    node->as.return_statement.value = value;
    return node;
}

//paser状态
typedef struct
{
    Lexer lexer;
    Token current;
} Parser;

//初始化parser
static void parser_init(Parser* parser, const char* source)
{
    lexer_init(&parser->lexer, source);
    parser->current = lexer_next_token(&parser->lexer);
}

//前进一个token
static void parser_advance(Parser *parser)
{
    parser->current = lexer_next_token(&parser->lexer);
}

//查看type是否相同，相同返回1，不同返回0
static int parser_check(const Parser *parser, TokenType type)
{
    return parser->current.type == type;
}

//如果匹配就使用
static int paser_match(Parser* parser,TokenType type)
{
    if(!parser_check(parser, type))
    {
        return 0;
    }
    parser_advance(parser);
    return 1;
}

//要求必须出现某个token
static Token parser_expect(Parser* parser, TokenType type, const char* message)
{
    if (!parser_check(parser, type))
    {
        fprintf(stderr,
        "error line: %d, column: %d. %s: now token is %s\n",
            parser->current.line, parser->current.column, message,
            token_type_name(parser->current.type));
        exit(EXIT_FAILURE);
    }

    Token token = parser->current;
    parser_advance(parser);
    return token;
}

//链表初始化
static ASTNodeList *ast_list_new(ASTNode *node)
{
    ASTNodeList *item = malloc(sizeof(ASTNodeList));

    if (item == NULL)
    {
        fprintf(stderr, "dont have enough memory\n");
        exit(EXIT_FAILURE);
    }
    item->node = node;
    item->next = NULL;
    return item;
}

static void ast_list_append(ASTNodeList **head, ASTNode *node)
{
    ASTNodeList *item = ast_list_new(node);
    if (*head == NULL)
    {
        *head = item;
        return;
    }
    ASTNodeList* current = *head;
    //将node添加到head的最后
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = item;
}
//ast_list_append(&block->as.block.statements, statement);










static ASTNode* parser_primary(Parser* parser)
{
    if (parser_check(parser, TOKEN_NUMBER))
    {
        Token number_token = parser->current;
        parser_advance(parser);
        return 1;
    }
}

static ASTNode* parser_expression(Parser* parser);


char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "无法打开文件：%s\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *source = malloc((size_t)size + 1);

    if (source == NULL)
    {
        fprintf(stderr, "内存分配失败\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t read_size = fread(source, 1, (size_t)size, file);
    source[read_size] = '\0';

    fclose(file);
    return source;
}

// int main(int argc, char **argv)
// {
//     if (argc < 2)
//     {
//         fprintf(stderr, "用法: minic <source.mc>\n");
//         return 1;
//     }
//     const char *filename = argv[1];
// }

static const char *token_type_name(TokenType type);
int main()
{
    const char* source = 
        "int main() {\n"
        "    int n;\n"
        "    n = 17;\n"
        "\n"
        "    if (n >= 2) {\n"
        "        return n % 2;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n";

    Lexer lexer;
    lexer_init(&lexer, source);
    for (;;)
    {
        Token token = lexer_next_token(&lexer);
        printf(
            "%-22s text=\"%s\" value=%d "
            "line and column=%d:%d\n",
            token_type_name(token.type),
            token.text,
            token.value,
            token.line,
            token.column
        );
        if (token.type == TOKEN_EOF) break;

    }
    return 0;

}
static const char *token_type_name(TokenType type)
{
    switch (type) {
        case TOKEN_EOF:
            return "TOKEN_EOF";

        case TOKEN_INT:
            return "TOKEN_INT";

        case TOKEN_RETURN:
            return "TOKEN_RETURN";

        case TOKEN_IF:
            return "TOKEN_IF";

        case TOKEN_ELSE:
            return "TOKEN_ELSE";

        case TOKEN_WHILE:
            return "TOKEN_WHILE";

        case TOKEN_IDENTIFIER:
            return "TOKEN_IDENTIFIER";

        case TOKEN_NUMBER:
            return "TOKEN_NUMBER";

        case TOKEN_PLUS:
            return "TOKEN_PLUS";

        case TOKEN_MINUS:
            return "TOKEN_MINUS";

        case TOKEN_STAR:
            return "TOKEN_STAR";

        case TOKEN_SLASH:
            return "TOKEN_SLASH";

        case TOKEN_PERCENT:
            return "TOKEN_PERCENT";

        case TOKEN_ASSIGN:
            return "TOKEN_ASSIGN";

        case TOKEN_EQUAL:
            return "TOKEN_EQUAL";

        case TOKEN_NOT_EQUAL:
            return "TOKEN_NOT_EQUAL";

        case TOKEN_LESS:
            return "TOKEN_LESS";

        case TOKEN_LESS_EQUAL:
            return "TOKEN_LESS_EQUAL";

        case TOKEN_GREATER:
            return "TOKEN_GREATER";

        case TOKEN_GREATER_EQUAL:
            return "TOKEN_GREATER_EQUAL";

        case TOKEN_LPAREN:
            return "TOKEN_LPAREN";

        case TOKEN_RPAREN:
            return "TOKEN_RPAREN";

        case TOKEN_LBRACE:
            return "TOKEN_LBRACE";

        case TOKEN_RBRACE:
            return "TOKEN_RBRACE";

        case TOKEN_COMMA:
            return "TOKEN_COMMA";

        case TOKEN_SEMICOLON:
            return "TOKEN_SEMICOLON";

        default:
            return "TOKEN_UNKNOWN";
    }
}