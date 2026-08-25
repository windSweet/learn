/*
 * minic_interpreter.c - 一个单文件 MiniC 解释器
 *
 * 支持：
 *   int main() { ... }
 *   int 变量声明、赋值、整数、变量、括号
 *   + - * / %、== != < <= > >=、&& || !
 *   if/else、while、代码块、return
 *   print(表达式);                 // 内置输出函数
 *   // 行注释 和斜杠星号块注释
 *
 * 编译：gcc -std=c11 -Wall -Wextra -O2 minic_interpreter.c -o minic
 * 运行：minic test.minic
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 64
#define MAX_VARS 256
#define MAX_LOOP_ITERATIONS 1000000

typedef enum {
    TK_EOF, TK_NUMBER, TK_IDENT,
    TK_INT, TK_RETURN, TK_IF, TK_ELSE, TK_WHILE, TK_PRINT,
    TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH, TK_PERCENT,
    TK_ASSIGN, TK_EQ, TK_NE, TK_LT, TK_LE, TK_GT, TK_GE,
    TK_AND, TK_OR, TK_NOT,
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE,
    TK_SEMI, TK_COMMA
} TokenType;

typedef struct {
    TokenType type;
    long value;
    char text[MAX_NAME];
    int line;
    int column;
} Token;

//生成lexer里面存储token的位置和源代码
typedef struct {
    const char *source;
    size_t pos;
    int line;
    int column;
} Lexer;

static void fatal_at(int line, int column, const char *message)
{
    fprintf(stderr, "错误（第 %d 行，第 %d 列）：%s\n", line, column, message);
    exit(EXIT_FAILURE);
}
//返回lexer的位置
static char lexer_peek(const Lexer *lexer)
{
    return lexer->source[lexer->pos];
}

static char lexer_peek_next(const Lexer *lexer)
{
    char ch = lexer->source[lexer->pos];
    return ch ? lexer->source[lexer->pos + 1] : '\0';
}

static char lexer_advance(Lexer *lexer)
{
    char ch = lexer->source[lexer->pos++];
    if (ch == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return ch;
}

static void lexer_skip_space_and_comments(Lexer *lexer)
{
    for (;;) {
        while (isspace((unsigned char)lexer_peek(lexer)))
            lexer_advance(lexer);

        if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '/') {
            while (lexer_peek(lexer) && lexer_peek(lexer) != '\n')
                lexer_advance(lexer);
            continue;
        }

        if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '*') {
            int line = lexer->line;
            int column = lexer->column;
            lexer_advance(lexer);
            lexer_advance(lexer);
            while (lexer_peek(lexer) &&
                   !(lexer_peek(lexer) == '*' && lexer_peek_next(lexer) == '/'))
                lexer_advance(lexer);
            if (!lexer_peek(lexer))
                fatal_at(line, column, "块注释没有结束");
            lexer_advance(lexer);
            lexer_advance(lexer);
            continue;
        }
        break;
    }
}

static Token make_token(TokenType type, int line, int column)
{
    Token token;
    memset(&token, 0, sizeof(token));
    token.type = type;
    token.line = line;
    token.column = column;
    return token;
}

static Token lexer_next(Lexer *lexer)
{
    Token token;
    int line, column;
    char ch;

    lexer_skip_space_and_comments(lexer);
    line = lexer->line;
    column = lexer->column;
    ch = lexer_peek(lexer);

    if (!ch)
        return make_token(TK_EOF, line, column);

    if (isdigit((unsigned char)ch)) {
        long value = 0;
        token = make_token(TK_NUMBER, line, column);
        while (isdigit((unsigned char)lexer_peek(lexer))) {
            int digit = lexer_advance(lexer) - '0';
            value = value * 10 + digit;
        }
        token.value = value;
        return token;
    }

    if (isalpha((unsigned char)ch) || ch == '_') {
        size_t length = 0;
        token = make_token(TK_IDENT, line, column);
        while (isalnum((unsigned char)lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
            ch = lexer_advance(lexer);
            if (length + 1 < sizeof(token.text))
                token.text[length++] = ch;
        }
        token.text[length] = '\0';
        if (!strcmp(token.text, "int")) token.type = TK_INT;
        else if (!strcmp(token.text, "return")) token.type = TK_RETURN;
        else if (!strcmp(token.text, "if")) token.type = TK_IF;
        else if (!strcmp(token.text, "else")) token.type = TK_ELSE;
        else if (!strcmp(token.text, "while")) token.type = TK_WHILE;
        else if (!strcmp(token.text, "print")) token.type = TK_PRINT;
        return token;
    }

    lexer_advance(lexer);
    switch (ch) {
    case '+': return make_token(TK_PLUS, line, column);
    case '-': return make_token(TK_MINUS, line, column);
    case '*': return make_token(TK_STAR, line, column);
    case '/': return make_token(TK_SLASH, line, column);
    case '%': return make_token(TK_PERCENT, line, column);
    case '(': return make_token(TK_LPAREN, line, column);
    case ')': return make_token(TK_RPAREN, line, column);
    case '{': return make_token(TK_LBRACE, line, column);
    case '}': return make_token(TK_RBRACE, line, column);
    case ';': return make_token(TK_SEMI, line, column);
    case ',': return make_token(TK_COMMA, line, column);
    case '=':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TK_EQ, line, column);
        }
        return make_token(TK_ASSIGN, line, column);
    case '!':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TK_NE, line, column);
        }
        return make_token(TK_NOT, line, column);
    case '<':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TK_LE, line, column);
        }
        return make_token(TK_LT, line, column);
    case '>':
        if (lexer_peek(lexer) == '=') {
            lexer_advance(lexer);
            return make_token(TK_GE, line, column);
        }
        return make_token(TK_GT, line, column);
    case '&':
        if (lexer_peek(lexer) == '&') {
            lexer_advance(lexer);
            return make_token(TK_AND, line, column);
        }
        fatal_at(line, column, "只支持逻辑与 &&，不支持单独的 &");
        break;
    case '|':
        if (lexer_peek(lexer) == '|') {
            lexer_advance(lexer);
            return make_token(TK_OR, line, column);
        }
        fatal_at(line, column, "只支持逻辑或 ||，不支持单独的 |");
        break;
    default:
        fatal_at(line, column, "无法识别的字符");
    }
    return make_token(TK_EOF, line, column);
}

typedef enum {
    EX_NUMBER, EX_VARIABLE, EX_ASSIGN, EX_BINARY, EX_UNARY
} ExprType;

typedef struct Expr Expr;
struct Expr {
    ExprType type;
    TokenType op;
    long number;
    char name[MAX_NAME];
    Expr *left;
    Expr *right;
    int line;
    int column;
};

typedef enum {
    ST_BLOCK, ST_DECL, ST_EXPR, ST_PRINT,
    ST_IF, ST_WHILE, ST_RETURN
} StmtType;

typedef struct Stmt Stmt;
struct Stmt {
    StmtType type;
    char name[MAX_NAME];
    Expr *expr;
    Expr *condition;
    Stmt *body;
    Stmt *else_body;
    Stmt *first;
    Stmt *next;
    int line;
    int column;
};

typedef struct {
    Lexer lexer;
    Token current;
} Parser;

static void *checked_calloc(size_t count, size_t size)
{
    void *p = calloc(count, size);
    if (!p) {
        fprintf(stderr, "错误：内存不足\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

static Expr *new_expr(ExprType type, Token token)
{
    Expr *expr = checked_calloc(1, sizeof(*expr));
    expr->type = type;
    expr->line = token.line;
    expr->column = token.column;
    return expr;
}

static Stmt *new_stmt(StmtType type, Token token)
{
    Stmt *stmt = checked_calloc(1, sizeof(*stmt));
    stmt->type = type;
    stmt->line = token.line;
    stmt->column = token.column;
    return stmt;
}

static void parser_advance(Parser *parser)
{
    parser->current = lexer_next(&parser->lexer);
}

static int parser_match(Parser *parser, TokenType type)
{
    if (parser->current.type != type)
        return 0;
    parser_advance(parser);
    return 1;
}

static Token parser_expect(Parser *parser, TokenType type, const char *message)
{
    Token token = parser->current;
    if (token.type != type)
        fatal_at(token.line, token.column, message);
    parser_advance(parser);
    return token;
}

static Expr *parse_expression(Parser *parser);
static Stmt *parse_statement(Parser *parser);

static Expr *parse_primary(Parser *parser)
{
    Token token = parser->current;
    Expr *expr;

    if (parser_match(parser, TK_NUMBER)) {
        expr = new_expr(EX_NUMBER, token);
        expr->number = token.value;
        return expr;
    }
    if (parser_match(parser, TK_IDENT)) {
        expr = new_expr(EX_VARIABLE, token);
        strcpy(expr->name, token.text);
        return expr;
    }
    if (parser_match(parser, TK_LPAREN)) {
        expr = parse_expression(parser);
        parser_expect(parser, TK_RPAREN, "缺少右括号 ')'");
        return expr;
    }
    fatal_at(token.line, token.column, "这里需要数字、变量或括号表达式");
    return NULL;
}

static Expr *parse_unary(Parser *parser)
{
    Token token = parser->current;
    if (token.type == TK_MINUS || token.type == TK_PLUS || token.type == TK_NOT) {
        Expr *expr;
        parser_advance(parser);
        expr = new_expr(EX_UNARY, token);
        expr->op = token.type;
        expr->right = parse_unary(parser);
        return expr;
    }
    return parse_primary(parser);
}

static Expr *parse_binary_level(Parser *parser, Expr *(*next_level)(Parser *),
                                const TokenType *operators, size_t count)
{
    Expr *left = next_level(parser);
    for (;;) {
        size_t i;
        Token token = parser->current;
        for (i = 0; i < count; i++)
            if (token.type == operators[i]) break;
        if (i == count) break;
        parser_advance(parser);
        {
            Expr *expr = new_expr(EX_BINARY, token);
            expr->op = token.type;
            expr->left = left;
            expr->right = next_level(parser);
            left = expr;
        }
    }
    return left;
}

static Expr *parse_factor(Parser *parser)
{
    static const TokenType ops[] = {TK_STAR, TK_SLASH, TK_PERCENT};
    return parse_binary_level(parser, parse_unary, ops, 3);
}

static Expr *parse_term(Parser *parser)
{
    static const TokenType ops[] = {TK_PLUS, TK_MINUS};
    return parse_binary_level(parser, parse_factor, ops, 2);
}

static Expr *parse_comparison(Parser *parser)
{
    static const TokenType ops[] = {TK_LT, TK_LE, TK_GT, TK_GE};
    return parse_binary_level(parser, parse_term, ops, 4);
}

static Expr *parse_equality(Parser *parser)
{
    static const TokenType ops[] = {TK_EQ, TK_NE};
    return parse_binary_level(parser, parse_comparison, ops, 2);
}

static Expr *parse_and(Parser *parser)
{
    static const TokenType ops[] = {TK_AND};
    return parse_binary_level(parser, parse_equality, ops, 1);
}

static Expr *parse_or(Parser *parser)
{
    static const TokenType ops[] = {TK_OR};
    return parse_binary_level(parser, parse_and, ops, 1);
}

static Expr *parse_assignment(Parser *parser)
{
    Expr *left = parse_or(parser);
    if (parser->current.type == TK_ASSIGN) {
        Token token = parser->current;
        Expr *expr;
        parser_advance(parser);
        if (left->type != EX_VARIABLE)
            fatal_at(token.line, token.column, "赋值号左边必须是变量");
        expr = new_expr(EX_ASSIGN, token);
        strcpy(expr->name, left->name);
        free(left);
        expr->right = parse_assignment(parser);
        return expr;
    }
    return left;
}

static Expr *parse_expression(Parser *parser)
{
    return parse_assignment(parser);
}

static Stmt *parse_block(Parser *parser, Token opening)
{
    Stmt *block = new_stmt(ST_BLOCK, opening);
    Stmt **tail = &block->first;
    while (parser->current.type != TK_RBRACE) {
        if (parser->current.type == TK_EOF)
            fatal_at(opening.line, opening.column, "代码块缺少右花括号 '}'");
        *tail = parse_statement(parser);
        tail = &(*tail)->next;
    }
    parser_advance(parser);
    return block;
}

static Stmt *parse_statement(Parser *parser)
{
    Token token = parser->current;
    Stmt *stmt;

    if (parser_match(parser, TK_LBRACE))
        return parse_block(parser, token);

    if (parser_match(parser, TK_INT)) {
        Token name = parser_expect(parser, TK_IDENT, "int 后面需要变量名");
        stmt = new_stmt(ST_DECL, token);
        strcpy(stmt->name, name.text);
        if (parser_match(parser, TK_ASSIGN))
            stmt->expr = parse_expression(parser);
        parser_expect(parser, TK_SEMI, "变量声明后缺少分号 ';'");
        return stmt;
    }

    if (parser_match(parser, TK_PRINT)) {
        stmt = new_stmt(ST_PRINT, token);
        parser_expect(parser, TK_LPAREN, "print 后面缺少左括号 '('");
        stmt->expr = parse_expression(parser);
        parser_expect(parser, TK_RPAREN, "print 表达式后缺少右括号 ')'");
        parser_expect(parser, TK_SEMI, "print 语句后缺少分号 ';'");
        return stmt;
    }

    if (parser_match(parser, TK_RETURN)) {
        stmt = new_stmt(ST_RETURN, token);
        stmt->expr = parse_expression(parser);
        parser_expect(parser, TK_SEMI, "return 语句后缺少分号 ';'");
        return stmt;
    }

    if (parser_match(parser, TK_IF)) {
        stmt = new_stmt(ST_IF, token);
        parser_expect(parser, TK_LPAREN, "if 后面缺少左括号 '('");
        stmt->condition = parse_expression(parser);
        parser_expect(parser, TK_RPAREN, "if 条件后缺少右括号 ')'");
        stmt->body = parse_statement(parser);
        if (parser_match(parser, TK_ELSE))
            stmt->else_body = parse_statement(parser);
        return stmt;
    }

    if (parser_match(parser, TK_WHILE)) {
        stmt = new_stmt(ST_WHILE, token);
        parser_expect(parser, TK_LPAREN, "while 后面缺少左括号 '('");
        stmt->condition = parse_expression(parser);
        parser_expect(parser, TK_RPAREN, "while 条件后缺少右括号 ')'");
        stmt->body = parse_statement(parser);
        return stmt;
    }

    stmt = new_stmt(ST_EXPR, token);
    stmt->expr = parse_expression(parser);
    parser_expect(parser, TK_SEMI, "表达式后缺少分号 ';'");
    return stmt;
}

static Stmt *parse_program(Parser *parser)
{
    Token main_name;
    Stmt *program;
    parser_expect(parser, TK_INT, "程序必须以 int main() 开始");
    main_name = parser_expect(parser, TK_IDENT, "int 后面需要函数名 main");
    if (strcmp(main_name.text, "main"))
        fatal_at(main_name.line, main_name.column, "简易版目前只支持 main 函数");
    parser_expect(parser, TK_LPAREN, "main 后面缺少左括号 '('");
    parser_expect(parser, TK_RPAREN, "main 暂时不能包含参数");
    {
        Token opening = parser_expect(parser, TK_LBRACE, "main 后面需要代码块");
        program = parse_block(parser, opening);
    }
    parser_expect(parser, TK_EOF, "main 函数结束后还有多余内容");
    return program;
}

typedef struct {
    char name[MAX_NAME];
    long value;
    int depth;
} Variable;

typedef struct {
    Variable vars[MAX_VARS];
    int count;
    int depth;
    int returned;
    long return_value;
} Environment;

static int find_variable(const Environment *env, const char *name)
{
    int i;
    for (i = env->count - 1; i >= 0; i--)
        if (!strcmp(env->vars[i].name, name)) return i;
    return -1;
}

static long eval_expr(Expr *expr, Environment *env)
{
    long left, right;
    int index;
    switch (expr->type) {
    case EX_NUMBER:
        return expr->number;
    case EX_VARIABLE:
        index = find_variable(env, expr->name);
        if (index < 0)
            fatal_at(expr->line, expr->column, "使用了尚未声明的变量");
        return env->vars[index].value;
    case EX_ASSIGN:
        index = find_variable(env, expr->name);
        if (index < 0)
            fatal_at(expr->line, expr->column, "给尚未声明的变量赋值");
        env->vars[index].value = eval_expr(expr->right, env);
        return env->vars[index].value;
    case EX_UNARY:
        right = eval_expr(expr->right, env);
        if (expr->op == TK_MINUS) return -right;
        if (expr->op == TK_PLUS) return right;
        return !right;
    case EX_BINARY:
        /* 逻辑运算需要短路求值。 */
        left = eval_expr(expr->left, env);
        if (expr->op == TK_AND) return left && eval_expr(expr->right, env);
        if (expr->op == TK_OR) return left || eval_expr(expr->right, env);
        right = eval_expr(expr->right, env);
        switch (expr->op) {
        case TK_PLUS: return left + right;
        case TK_MINUS: return left - right;
        case TK_STAR: return left * right;
        case TK_SLASH:
            if (!right) fatal_at(expr->line, expr->column, "除数不能为零");
            return left / right;
        case TK_PERCENT:
            if (!right) fatal_at(expr->line, expr->column, "取模运算的除数不能为零");
            return left % right;
        case TK_EQ: return left == right;
        case TK_NE: return left != right;
        case TK_LT: return left < right;
        case TK_LE: return left <= right;
        case TK_GT: return left > right;
        case TK_GE: return left >= right;
        default: break;
        }
    }
    fatal_at(expr->line, expr->column, "未知表达式");
    return 0;
}

static void exec_stmt(Stmt *stmt, Environment *env);

static void exec_block(Stmt *block, Environment *env)
{
    Stmt *item;
    int old_count = env->count;
    env->depth++;
    for (item = block->first; item && !env->returned; item = item->next)
        exec_stmt(item, env);
    env->count = old_count;
    env->depth--;
}

static void exec_stmt(Stmt *stmt, Environment *env)
{
    int i;
    long iterations;
    switch (stmt->type) {
    case ST_BLOCK:
        exec_block(stmt, env);
        break;
    case ST_DECL:
        for (i = env->count - 1; i >= 0 && env->vars[i].depth == env->depth; i--)
            if (!strcmp(env->vars[i].name, stmt->name))
                fatal_at(stmt->line, stmt->column, "同一作用域中重复声明变量");
        if (env->count >= MAX_VARS)
            fatal_at(stmt->line, stmt->column, "变量数量超过上限");
        strcpy(env->vars[env->count].name, stmt->name);
        env->vars[env->count].depth = env->depth;
        env->vars[env->count].value = stmt->expr ? eval_expr(stmt->expr, env) : 0;
        env->count++;
        break;
    case ST_EXPR:
        (void)eval_expr(stmt->expr, env);
        break;
    case ST_PRINT:
        printf("%ld\n", eval_expr(stmt->expr, env));
        break;
    case ST_IF:
        if (eval_expr(stmt->condition, env)) exec_stmt(stmt->body, env);
        else if (stmt->else_body) exec_stmt(stmt->else_body, env);
        break;
    case ST_WHILE:
        iterations = 0;
        while (!env->returned && eval_expr(stmt->condition, env)) {
            if (++iterations > MAX_LOOP_ITERATIONS)
                fatal_at(stmt->line, stmt->column, "循环次数超过上限，可能存在死循环");
            exec_stmt(stmt->body, env);
        }
        break;
    case ST_RETURN:
        env->return_value = eval_expr(stmt->expr, env);
        env->returned = 1;
        break;
    }
}

static void free_expr(Expr *expr)
{
    if (!expr) return;
    free_expr(expr->left);
    free_expr(expr->right);
    free(expr);
}

static void free_stmt(Stmt *stmt)
{
    while (stmt) {
        Stmt *next = stmt->next;
        free_expr(stmt->expr);
        free_expr(stmt->condition);
        free_stmt(stmt->body);
        free_stmt(stmt->else_body);
        free_stmt(stmt->first);
        free(stmt);
        stmt = next;
    }
}

static char *read_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    long size;
    char *buffer;
    size_t got;
    if (!file) {
        fprintf(stderr, "错误：无法打开文件 %s\n", path);
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "错误：无法读取文件大小 %s\n", path);
        fclose(file);
        return NULL;
    }
    buffer = checked_calloc((size_t)size + 1, 1);
    got = fread(buffer, 1, (size_t)size, file);
    if (got != (size_t)size && ferror(file)) {
        fprintf(stderr, "错误：读取文件失败 %s\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }
    buffer[got] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char **argv)
{
    char *source;
    Parser parser;
    Stmt *program;
    Environment env;

    if (argc != 2) {
        fprintf(stderr, "用法：%s <源文件.minic>\n", argv[0]);
        return EXIT_FAILURE;
    }
    source = read_file(argv[1]);
    if (!source) return EXIT_FAILURE;

    memset(&parser, 0, sizeof(parser));
    parser.lexer.source = source;
    parser.lexer.line = 1;
    parser.lexer.column = 1;
    parser_advance(&parser);
    program = parse_program(&parser);

    memset(&env, 0, sizeof(env));
    exec_stmt(program, &env);
    if (!env.returned)
        fprintf(stderr, "警告：main 没有 return，按 return 0 处理\n");

    free_stmt(program);
    free(source);
    return env.returned ? (int)(env.return_value & 0xff) : 0;
}
