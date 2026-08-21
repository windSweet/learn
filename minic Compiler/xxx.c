#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>   // for strcasecmp

#define MAX_MACROS     100
#define MAX_NAME_LEN   64
#define MAX_REPL_LEN   1024
#define MAX_LINE_LEN   4096

typedef struct {
    char name[MAX_NAME_LEN + 1];
    char repl[MAX_REPL_LEN + 1];
} Macro;

Macro macros[MAX_MACROS];
int macro_count = 0;

/* 判断字符是否为 C 标识符的一部分（字母、数字、下划线） */
static int is_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

/* 跳过空白字符（空格和制表符） */
static void skip_whitespace(const char **p) {
    while (**p == ' ' || **p == '\t') (*p)++;
}

/* 读取一个标识符（由字母、数字、下划线组成）到 dest，返回读取的长度 */
static int read_identifier(const char *src, char *dest, int max_len) {
    int len = 0;
    while (is_ident_char(*src) && len < max_len) {
        dest[len++] = *src++;
    }
    dest[len] = '\0';
    return len;
}

/* 处理一行，进行宏替换并输出到 stdout */
static void process_line(const char *line) {
    const char *p = line;
    while (*p) {
        if (is_ident_char(*p)) {
            const char *start = p;
            char name[MAX_NAME_LEN + 1];
            int len = read_identifier(p, name, MAX_NAME_LEN);
            p += len;   // p now points to first non‑identifier character

            // 检查分隔符：空格、制表符、换行或字符串结束
            if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\0') {
                int idx = -1;
                for (int i = 0; i < macro_count; i++) {
                    if (strcmp(macros[i].name, name) == 0) {
                        idx = i;
                        break;
                    }
                }
                if (idx != -1) {
                    fputs(macros[idx].repl, stdout);
                    // 输出分隔符（如果有），然后跳过它
                    if (*p) {
                        putchar(*p);
                        p++;
                    }
                    continue;
                }
            }
            // 未找到宏或分隔符不符合要求 → 原样输出标识符
            fwrite(start, 1, len, stdout);
            // 注意：此时 p 已经指向分隔符，循环下轮会处理该字符
        } else {
            putchar(*p);
            p++;
        }
    }
}

int main(void) {
    char line[MAX_LINE_LEN + 1];

    while (fgets(line, sizeof(line), stdin)) {
        // 去掉末尾换行符（保留以便后续判断文件结束）
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // 检查该行是否以 '#' 开头（忽略前导空格）
        const char *p = line;
        skip_whitespace(&p);
        if (*p == '#') {
            p++;  // 跳过 '#'
            skip_whitespace(&p);

            // 读取关键字，判断是否为 define（不区分大小写）
            char keyword[16];
            int kw_len = read_identifier(p, keyword, sizeof(keyword) - 1);
            if (kw_len == 0 || strcasecmp(keyword, "define") != 0) {
                fprintf(stderr, "Error: unsupported preprocessing directive '%.*s'\n",
                        kw_len, keyword);
                exit(EXIT_FAILURE);
            }
            p += kw_len;   // 跳过关键字
            skip_whitespace(&p);

            // 读取宏名
            if (!is_ident_char(*p)) {
                fprintf(stderr, "Error: expected macro name after #define\n");
                exit(EXIT_FAILURE);
            }
            char mname[MAX_NAME_LEN + 1];
            int name_len = read_identifier(p, mname, MAX_NAME_LEN);
            p += name_len;
            if (name_len > MAX_NAME_LEN) {
                fprintf(stderr, "Error: macro name too long\n");
                exit(EXIT_FAILURE);
            }

            // 跳过宏名后的空白，剩余部分作为替换文本（保留内部空白）
            skip_whitespace(&p);
            char repl[MAX_REPL_LEN + 1];
            int repl_len = 0;
            while (*p && repl_len < MAX_REPL_LEN) {
                repl[repl_len++] = *p++;
            }
            repl[repl_len] = '\0';

            // 存入宏表
            if (macro_count >= MAX_MACROS) {
                fprintf(stderr, "Error: too many #define macros (max %d)\n", MAX_MACROS);
                exit(EXIT_FAILURE);
            }
            strcpy(macros[macro_count].name, mname);
            strcpy(macros[macro_count].repl, repl);
            macro_count++;

            // 跳过这一行（不输出）
            continue;
        }

        // 普通行 → 进行宏替换并输出
        process_line(line);
        putchar('\n');   // 恢复换行
    }

    return 0;
}