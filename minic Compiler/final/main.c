#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_MACROS 100
#define MAX_NAME_LEN 63
#define MAX_VALUE_LEN 1023
#define MAX_LINE_LEN 4096

//枚举
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

//节点的链表
typedef struct ASTNodeList {
    ASTNode *node;
    struct ASTNodeList *next;
} ASTNodeList;


//ast的节点类型
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

//不同的ast节点
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

//确认是否是字母或者数字
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

//实现对lexer的更新,这里的意思就是lexer并不是对整个文本分析完后再进行处理
//是对每个token单独分析
//parser里面会有对lexer的更新，意思就是走一步看一步
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

//创建双字符token，这里针对的是==。!= 这种
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
    Token token = make_token(TOKEN_IDENTIFIER, lexer->line, lexer->column);
    size_t length = 0;

    token.line = lexer->line;
    token.column = lexer->column;
    //isalnum需要强制转换unsigned char
    while(
        isalnum((unsigned char)lexer_current(lexer)) ||
        lexer_current(lexer) == '_'
    )
    {
        if (length < sizeof(token.text) - 1)
        {
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

//parser部分

//链表初始化
static ASTNodeList *ast_list_new(ASTNode *node)
{
    //malloc返回一个万能指针，然后强转
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
    //这里用双指针是因为最开始创建这个链表时候肯定有个null链表
    //这时候我们需要把这个链表的指针指针指向这个新建的item链表
    //将node打包成item
    ASTNodeList *item = ast_list_new(node);
    if (*head == NULL)
    {
        //如果head为空链表，那么item当头链表
        *head = item;
        return;
    }
    ASTNodeList* current = *head;
    //将node追加到head的最后
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = item;
}
//ast_list_append(&block->as.block.statements, statement);


//创建ast节点
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
static ASTNode *ast_new_if(Token if_token, ASTNode *condition, ASTNode *then_branch, ASTNode *else_branch)
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

//创建函数节点
static ASTNode *ast_new_function(Token int_token, Token name_token, ASTNodeList *parameters, ASTNode *body)
{
    ASTNode *node = ast_new(AST_FUNCTION, int_token);

    snprintf(node->as.function.name,
        sizeof(node->as.function.name),"%s",name_token.text);

    node->as.function.parameters = parameters;
    node->as.function.body = body;
    return node;
}

//创建函数调用表达式节点，例如 input()、print(candidate)
static ASTNode *ast_new_call(Token name_token, ASTNodeList *arguments)
{
    ASTNode *node = ast_new(AST_CALL, name_token);
    snprintf(node->as.call.name, sizeof(node->as.call.name), "%s", name_token.text);
    node->as.call.arguments = arguments;
    return node;
}

//构建函数 将已经创建好的表达式包装成语句。
static ASTNode *ast_new_expr_stmt(ASTNode *expression)
{
    if (expression == NULL)
    {
        fprintf(stderr, "cant null\n");
        exit(EXIT_FAILURE);
    }
    ASTNode *node = ast_new(AST_EXPR_STMT, expression->token);
    node->as.expression_statement.expression = expression;
    return node;
}

//创建程序节点
static ASTNode *ast_new_program(Token first_token)
{
    ASTNode *node =
        ast_new(AST_PROGRAM, first_token);

    node->as.program.functions = NULL;

    return node;
}

//向程序添加函数
static void ast_program_add_function(ASTNode *program, ASTNode *function)
{
    if (
        program == NULL ||
        program->type != AST_PROGRAM
    ) {
        fprintf(
            stderr,
            "target node isnt program node\n"
        );

        exit(EXIT_FAILURE);
    }

    ast_list_append(
        &program->as.program.functions,
        function
    );
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

//parser 的报错
static void parser_error(const Parser *parser, const char *message)
{
    fprintf(
        stderr,
        "error:line: %d, column: %d. %s:now Token is %s,content \"%s\"\n",
        parser->current.line,
        parser->current.column,
        message,
        token_type_name(parser->current.type),
        parser->current.text
    );

    exit(EXIT_FAILURE);
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

//如果token匹配就使用
static int parser_match(Parser* parser,TokenType type)
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

//声明表达式解析的入口
static ASTNode *parse_expression(Parser *parser);

//调用函数
static ASTNode *parse_call_after_name(Parser *parser, Token name_token)
{
    ASTNodeList *arguments = NULL;

    /*
     * input() 当前直接是右括号，没有参数。
     * print(num) 当前是 num，需要解析参数。
     */
    if (!parser_check(parser, TOKEN_RPAREN))
    {
        for (;;)
        {
            ASTNode *argument = parse_expression(parser);

            ast_list_append(&arguments, argument);
            if (!parser_match(parser, TOKEN_COMMA))
            {
                break;
            }
        }
    }

    parser_expect(
        parser,
        TOKEN_RPAREN,
        "函数调用缺少 ')'"
    );

    return ast_new_call(
        name_token,
        arguments
    );
}

//声明变量解析
static ASTNode *parse_variable_declaration(Parser *parser)
{
    Token int_token = parser_expect(
        parser,
        TOKEN_INT,
        "变量声明需要以 int 开始"
    );

    Token name_token = parser_expect(
        parser,
        TOKEN_IDENTIFIER,
        "int 后面需要变量名"
    );

    ASTNode *initializer = NULL;

    /*
     * 顺便支持：
     *
     * int x = 10;
     */
    if (parser_match(parser, TOKEN_ASSIGN))
    {
        initializer =
            parse_expression(parser);
    }

    parser_expect(
        parser,
        TOKEN_SEMICOLON,
        "变量声明后缺少 ';'"
    );

    return ast_new_var_decl(
        int_token,
        name_token,
        initializer
    );
}

//解析数字和括号
static ASTNode *parser_primary(Parser *parser)
{
    if (parser_check(parser, TOKEN_NUMBER))
    {
        Token token = parser->current;
        parser_advance(parser);
        return ast_new_number(token);
    }
    
    //解析函数调用
    if (parser_check(parser, TOKEN_IDENTIFIER))
    {
        Token name_token = parser->current;
        parser_advance(parser);
        /*
         * 标识符后出现 '('，表示函数调用。
         */
        if (parser_match(parser, TOKEN_LPAREN))
        {
            return parse_call_after_name(parser, name_token);
        }
        return ast_new_variable(name_token);
    }

    //解析括号
    if (parser_match(parser, TOKEN_LPAREN))
    {
        //递归调用
        ASTNode *expression = parse_expression(parser);
        parser_expect(parser, TOKEN_RPAREN,"LPAREN dont have ')'");
        return expression;
    }

    fprintf(
        stderr,
        "errror: line:%d , column: %d"
        "need number or PAREN\n",
        parser->current.line,
        parser->current.column
    );

    exit(EXIT_FAILURE);
}
//解析乘法、除法、取模
static ASTNode *parse_multiplicative(Parser *parser)
{
    //左节点为left
    ASTNode *left = parser_primary(parser);

    while (
        parser_check(parser, TOKEN_STAR) ||
        parser_check(parser, TOKEN_SLASH) ||
        parser_check(parser, TOKEN_PERCENT)
    ) {
        Token operator_token = parser->current;
        parser_advance(parser);
        ASTNode *right = parser_primary(parser);
        left = ast_new_binary(operator_token, left, right);
    }
    //返回一个乘除取模的节点
    return left;
}
//解析加法和减法
static ASTNode *parse_additive(Parser *parser)
{
    //乘除优先级高于加减,所以先返回乘除得到的左节点
    ASTNode *left = parse_multiplicative(parser);

    while (
        parser_check(parser, TOKEN_PLUS) ||
        parser_check(parser, TOKEN_MINUS)
    ) {
        //把加减存到operator_token当中
        Token operator_token = parser->current;
        //更新parser
        parser_advance(parser);
        //右节点
        ASTNode *right = parse_multiplicative(parser);

        //创建二元表达式节点
        left = ast_new_binary(operator_token, left, right);
    }

    return left;
}
//解析比较运算
static ASTNode *parse_comparison(Parser *parser)
{
    ASTNode *left = parse_additive(parser);

    while (
        parser_check(parser, TOKEN_LESS) ||
        parser_check(parser, TOKEN_LESS_EQUAL) ||
        parser_check(parser, TOKEN_GREATER) ||
        parser_check(parser, TOKEN_GREATER_EQUAL)
    )
    {
        Token operator_token = parser->current;
        parser_advance(parser);
        ASTNode *right = parse_additive(parser);

        left = ast_new_binary(operator_token, left, right);
    }

    return left;
}
//解析相等运算
static ASTNode *parse_equality(Parser *parser)
{
    ASTNode *left = parse_comparison(parser);

    while (
        parser_check(parser, TOKEN_EQUAL) ||
        parser_check(parser, TOKEN_NOT_EQUAL)
    )
    {
        Token operator_token = parser->current;

        parser_advance(parser);

        ASTNode *right = parse_comparison(parser);

        left = ast_new_binary(operator_token, left, right);
    }

    return left;
}

//表达式入口
/*
parse_expression
└── parse_equality                 == !=
    └── parse_comparison           < <= > >=
        └── parse_additive         + -
            └── parse_multiplicative  * / %
                └── parser_primary
*/
static ASTNode *parse_expression(Parser *parser)
{
    return parse_equality(parser);
}

//实现return
static ASTNode *parse_return_statement(Parser *parser)
{
    Token return_token =parser_expect(parser, TOKEN_RETURN, "need return");
    ASTNode *value = parse_expression(parser);
    parser_expect(parser, TOKEN_SEMICOLON, "return dont have ';'");
    return ast_new_return(return_token, value);
}

static ASTNode *parse_block(Parser *parser);


//判断标识符
static ASTNode *parse_identifier_statement(Parser *parser)
{
    //判断开头是表示符
    Token name_token = parser_expect(parser, TOKEN_IDENTIFIER, "need identifier");
    //赋值语句
    if (parser_check(parser, TOKEN_ASSIGN))
    {
        //保存赋值token
        Token assign_token = parser->current;
        //消费=
        parser_advance(parser);
        //把赋值左侧的变量名创建为 AST_VARIABLE。
        ASTNode *target = ast_new_variable(name_token);
        //解析赋值右侧表达式。
        ASTNode *value = parse_expression(parser);
        parser_expect(parser, TOKEN_SEMICOLON, "dont have ';'");
        return ast_new_assignment(assign_token, target, value);
    }
    //函数调用语句
    if (parser_match(parser, TOKEN_LPAREN))
    {
        ASTNode *call =parse_call_after_name(parser, name_token);
        parser_expect(parser, TOKEN_SEMICOLON, "dont have ';'");
        return ast_new_expr_stmt(call);
    }

    parser_error(parser, "标识符后面需要 '=' 或 '('");

    return NULL;
}

//if 解析
static ASTNode *parse_if_statement(Parser *parser)
{
    Token if_token = parser_expect(
        parser,
        TOKEN_IF,
        "这里需要 if"
    );

    parser_expect(
        parser,
        TOKEN_LPAREN,
        "if 后缺少 '('"
    );

    ASTNode *condition =
        parse_expression(parser);

    parser_expect(
        parser,
        TOKEN_RPAREN,
        "if 条件后缺少 ')'"
    );

    ASTNode *then_branch =
        parse_block(parser);

    ASTNode *else_branch = NULL;

    if (parser_match(parser, TOKEN_ELSE))
    {
        else_branch =
            parse_block(parser);
    }

    return ast_new_if(
        if_token,
        condition,
        then_branch,
        else_branch
    );
}

//while 解析
static ASTNode *parse_while_statement(Parser *parser)
{
    Token while_token = parser_expect(
        parser,
        TOKEN_WHILE,
        "这里需要 while"
    );

    parser_expect(
        parser,
        TOKEN_LPAREN,
        "while 后缺少 '('"
    );

    ASTNode *condition =
        parse_expression(parser);

    parser_expect(
        parser,
        TOKEN_RPAREN,
        "while 条件后缺少 ')'"
    );

    ASTNode *body =
        parse_block(parser);

    return ast_new_while(
        while_token,
        condition,
        body
    );
}





//parser核心
static ASTNode *parse_statement(Parser *parser)
{
    if (parser_check(parser, TOKEN_INT))
    {
        return parse_variable_declaration(parser);
    }

    if (parser_check(parser, TOKEN_IF))
    {
        return parse_if_statement(parser);
    }

    if (parser_check(parser, TOKEN_WHILE))
    {
        return parse_while_statement(parser);
    }

    if (parser_check(parser, TOKEN_RETURN))
    {
        return parse_return_statement(parser);
    }

    if (parser_check(parser, TOKEN_LBRACE))
    {
        return parse_block(parser);
    }

    /*
     * 剩下的语句以表达式开始，例如：
     *
     * a = input();
     * candidate = candidate + 1;
     * print(candidate);
     */
    if (parser_check(parser, TOKEN_IDENTIFIER))
    {
        return parse_identifier_statement(parser);
    }

    parser_error(
        parser,
        "无法识别的语句"
    );

    return NULL;
}
//实现代码块
static ASTNode *parse_block(Parser *parser)
{
    Token left_brace =parser_expect(parser, TOKEN_LBRACE, "need '{'");
    ASTNode *block = ast_new_block(left_brace);
    while (
        !parser_check(parser, TOKEN_RBRACE) &&
        !parser_check(parser, TOKEN_EOF)
    )
    //只要还没有遇到 }，就继续解析代码块中的语句。
    {
        ASTNode *statement = parse_statement(parser);
        ast_block_add_statement(block, statement);
    }

    parser_expect(parser, TOKEN_RBRACE, "dont have }");
    return block;
}

//实现无参数的函数解析
static ASTNode *parse_function(Parser *parser)
{
    Token int_token = parser_expect(parser, TOKEN_INT, "function need int start");

    Token name_token = parser_expect(parser, TOKEN_IDENTIFIER, "int + function name");
    parser_expect(parser, TOKEN_LPAREN, "function need '('");
    parser_expect(parser, TOKEN_RPAREN, "dont have arag ')'");

    ASTNode *body = parse_block(parser);
    return ast_new_function(int_token, name_token, NULL, body);
}

//解析程序
static ASTNode *parse_program(Parser *parser)
{
    ASTNode *program = ast_new_program(parser->current);
    while (!parser_check(parser, TOKEN_EOF))
    {
        ASTNode *function = parse_function(parser);
        ast_program_add_function(program, function);
    }
    return program;
}

//分析astnode类型
static const char *ast_node_type_name(ASTNodeType type)
{
    switch (type)
    {
        case AST_PROGRAM: return "PROGRAM";
        case AST_FUNCTION: return "FUNCTION";
        case AST_BLOCK: return "BLOCK";
        case AST_RETURN: return "RETURN";
        case AST_BINARY: return "BINARY";
        case AST_NUMBER: return "NUMBER";
        case AST_VAR_DECL: return "VAR_DELC";
        case AST_ASSIGN: return "ASSIGN";
        case AST_IF: return "IF";
        case AST_WHILE: return "WHILE";
        case AST_VARIABLE: return "VARIABLE";
        case AST_CALL: return "CALL";
        default: return "UNKNOWN_AST";
    }
}









static void ast_print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

static void ast_print_list(
    const ASTNodeList *list,
    int indent
);

static void ast_print(
    const ASTNode *node,
    int indent
)
{
    if (node == NULL)
    {
        ast_print_indent(indent);
        printf("NULL\n");
        return;
    }

    ast_print_indent(indent);

    switch (node->type)
    {
        case AST_PROGRAM:
            printf("PROGRAM\n");
            ast_print_list(
                node->as.program.functions,
                indent + 1
            );
            break;

        case AST_FUNCTION:
            printf(
                "FUNCTION %s\n",
                node->as.function.name
            );
            ast_print(
                node->as.function.body,
                indent + 1
            );
            break;

        case AST_BLOCK:
            printf("BLOCK\n");
            ast_print_list(
                node->as.block.statements,
                indent + 1
            );
            break;

        case AST_VAR_DECL:
            printf(
                "VAR_DECL %s\n",
                node->as.variable_declaration.name
            );

            if (
                node->as.variable_declaration.initializer
                != NULL
            )
            {
                ast_print(
                    node->as.variable_declaration.initializer,
                    indent + 1
                );
            }
            break;

        case AST_ASSIGN:
            printf("ASSIGN\n");

            ast_print(
                node->as.assignment.target,
                indent + 1
            );

            ast_print(
                node->as.assignment.value,
                indent + 1
            );
            break;

        case AST_IF:
            printf("IF\n");

            ast_print_indent(indent + 1);
            printf("CONDITION\n");
            ast_print(
                node->as.if_statement.condition,
                indent + 2
            );

            ast_print_indent(indent + 1);
            printf("THEN\n");
            ast_print(
                node->as.if_statement.then_branch,
                indent + 2
            );

            if (node->as.if_statement.else_branch != NULL)
            {
                ast_print_indent(indent + 1);
                printf("ELSE\n");
                ast_print(
                    node->as.if_statement.else_branch,
                    indent + 2
                );
            }
            break;

        case AST_WHILE:
            printf("WHILE\n");

            ast_print_indent(indent + 1);
            printf("CONDITION\n");
            ast_print(
                node->as.while_statement.condition,
                indent + 2
            );

            ast_print_indent(indent + 1);
            printf("BODY\n");
            ast_print(
                node->as.while_statement.body,
                indent + 2
            );
            break;

        case AST_RETURN:
            printf("RETURN\n");
            ast_print(
                node->as.return_statement.value,
                indent + 1
            );
            break;

        case AST_EXPR_STMT:
            printf("EXPR_STMT\n");
            ast_print(
                node->as.expression_statement.expression,
                indent + 1
            );
            break;

        case AST_BINARY:
            printf(
                "BINARY %s\n",
                node->token.text
            );

            ast_print(
                node->as.binary.left,
                indent + 1
            );

            ast_print(
                node->as.binary.right,
                indent + 1
            );
            break;

        case AST_VARIABLE:
            printf(
                "VARIABLE %s\n",
                node->as.variable.name
            );
            break;

        case AST_NUMBER:
            printf(
                "NUMBER %ld\n",
                node->as.number.value
            );
            break;

        case AST_CALL:
            printf(
                "CALL %s\n",
                node->as.call.name
            );

            ast_print_list(
                node->as.call.arguments,
                indent + 1
            );
            break;

        default:
            printf("UNKNOWN_AST\n");
            break;
    }
}

static void ast_print_list(
    const ASTNodeList *list,
    int indent
)
{
    const ASTNodeList *current = list;

    while (current != NULL)
    {
        ast_print(current->node, indent);
        current = current->next;
    }
}
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

// int main()
// {
//     const char* source = 
//         "int main() {\n"
//         "    int n;\n"
//         "    n = 17;\n"
//         "\n"
//         "    if (n >= 2) {\n"
//         "        return n % 2;\n"
//         "    } else {\n"
//         "        return 0;\n"
//         "    }\n"
//         "}\n";

//     Lexer lexer;
//     lexer_init(&lexer, source);
//     for (;;)
//     {
//         Token token = lexer_next_token(&lexer);
//         printf(
//             "%-22s text=\"%s\" value=%d "
//             "line and column=%d:%d\n",
//             token_type_name(token.type),
//             token.text,
//             token.value,
//             token.line,
//             token.column
//         );
//         if (token.type == TOKEN_EOF) break;

//     }
//     return 0;

// }

int main(void)
{
    const char *source =
        "int main() {\n"
        "    int x;\n"
        "    int i;\n"
        "    int num;\n"
        "    int state;\n"
        "    int j;\n"
        "\n"
        "    x = input();\n"
        "    i = 0;\n"
        "    num = 1;\n"
        "    state = 1;\n"
        "    j = 2;\n"
        "\n"
        "    while (i < x) {\n"
        "        state = 1;\n"
        "        num = num + 1;\n"
        "        j = 2;\n"
        "\n"
        "        while (j * j <= num) {\n"
        "            if (num % j == 0) {\n"
        "                state = 0;\n"
        "            }\n"
        "\n"
        "            j = j + 1;\n"
        "        }\n"
        "\n"
        "        if (state == 1) {\n"
        "            i = i + 1;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    print(num);\n"
        "    return 0;\n"
        "}\n";

    Parser parser;
    parser_init(&parser, source);

    ASTNode *program =
        parse_program(&parser);

    ast_print(program, 0);

    return 0;
}