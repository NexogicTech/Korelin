// klexer.c
// Created by Helix on 2025/12/28.
//

#include "klexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 辅助函数：将 lexer 的指针向前移动一位
static void advance(KorelinLexer* lexer) {
    if (lexer->read_position >= strlen(lexer->input)) {
        lexer->current_char = '\0'; // 到达文件末尾
    } else {
        lexer->current_char = lexer->input[lexer->read_position];
    }
    lexer->position = lexer->read_position;
    lexer->read_position += 1;
}

// 辅助函数：查看下一个字符，但不移动指针
static char peek(KorelinLexer* lexer) {
    if (lexer->read_position >= strlen(lexer->input)) {
        return '\0';
    }
    return lexer->input[lexer->read_position];
}

// 辅助函数：跳过所有空白字符
static void skip_whitespace(KorelinLexer* lexer) {
    while (lexer->current_char == ' ' || lexer->current_char == '\t' ||
           lexer->current_char == '\n' || lexer->current_char == '\r') {
        advance(lexer);
    }
}

// 辅助函数：创建一个静态分配的Token（用于单/双字符）
static KorelinToken new_token(KorelinTokenType type, const char* value, size_t length) {
    return (KorelinToken){.type = type, .value = value, .length = length};
}

// 【问题二修复】关键字查找函数
KorelinTokenType lookup_ident(const char* ident) {
    if (strcmp(ident, "let") == 0) return KORELIN_LET;
    if (strcmp(ident, "var") == 0) return KORELIN_VAR;
    if (strcmp(ident, "const") == 0) return KORELIN_CONST;
    if (strcmp(ident, "func") == 0) return KORELIN_FUNC;
    if (strcmp(ident, "return") == 0) return KORELIN_RETURN;
    if (strcmp(ident, "if") == 0) return KORELIN_IF;
    if (strcmp(ident, "else") == 0) return KORELIN_ELSE;
    if (strcmp(ident, "elseif") == 0) return KORELIN_ELSEIF;
    if (strcmp(ident, "for") == 0) return KORELIN_FOR;
    if (strcmp(ident, "while") == 0) return KORELIN_WHILE;
    if (strcmp(ident, "break") == 0) return KORELIN_BREAK;
    if (strcmp(ident, "continue") == 0) return KORELIN_CONTINUE;
    if (strcmp(ident, "true") == 0) return KORELIN_TRUE;
    if (strcmp(ident, "false") == 0) return KORELIN_FALSE;
    if (strcmp(ident, "class") == 0) return KORELIN_CLASS;
    if (strcmp(ident, "struct") == 0) return KORELIN_STRUCT;
    if (strcmp(ident, "import") == 0) return KORELIN_IMPORT;
    if (strcmp(ident, "static") == 0) return KORELIN_STATIC;
    if (strcmp(ident, "public") == 0) return KORELIN_PUBLIC;
    if (strcmp(ident, "protected") == 0) return KORELIN_PROTECTED;
    if (strcmp(ident, "private") == 0) return KORELIN_PRIVATE;
    if (strcmp(ident, "int") == 0) return KORELIN_TYPE_INT;
    if (strcmp(ident, "long") == 0) return KORELIN_TYPE_LONG;
    if (strcmp(ident, "double") == 0) return KORELIN_TYPE_DOUBLE;
    if (strcmp(ident, "string") == 0) return KORELIN_TYPE_STRING;
    if (strcmp(ident, "bool") == 0) return KORELIN_TYPE_BOOL;
    return KORELIN_IDENT;
}

// 辅助函数：读取一个完整的标识符
static KorelinToken read_identifier(KorelinLexer* lexer) {
    size_t start_pos = lexer->position;
    while (isalpha(lexer->current_char) || lexer->current_char == '_' || isdigit(lexer->current_char)) {
        advance(lexer);
    }
    size_t len = lexer->position - start_pos;
    char* literal = malloc(len + 1);
    if (!literal) {
        fprintf(stderr, "Error: malloc failed in read_identifier\n");
        exit(EXIT_FAILURE);
    }
    strncpy(literal, lexer->input + start_pos, len);
    literal[len] = '\0';

    // 【问题二修复】调用 lookup_ident 来确定最终的Token类型
    KorelinTokenType type = lookup_ident(literal);
    KorelinToken token = {.type = type, .value = literal, .length = len};
    return token;
}

// 辅助函数：读取一个完整的数字
static KorelinToken read_number(KorelinLexer* lexer) {
    size_t start_pos = lexer->position;
    while (isdigit(lexer->current_char)) {
        advance(lexer);
    }
    size_t len = lexer->position - start_pos;
    char* literal = malloc(len + 1);
    if (!literal) {
        fprintf(stderr, "Error: malloc failed in read_number\n");
        exit(EXIT_FAILURE);
    }
    strncpy(literal, lexer->input + start_pos, len);
    literal[len] = '\0';

    // 【问题一修复】使用 KORELIN_INT
    KorelinToken token = {.type = KORELIN_INT, .value = literal, .length = len};
    return token;
}

// 【问题三修复】核心函数：获取下一个 Token
KorelinToken next_korelin_token(KorelinLexer* lexer) {
    skip_whitespace(lexer);

    switch (lexer->current_char) {
        // 单字符运算符和分隔符
        case '=': return new_token(KORELIN_ASSIGN, (const char[]){'=', '\0'}, 1);
        case '+': return new_token(KORELIN_ADD, (const char[]){'+', '\0'}, 1);
        case '-': return new_token(KORELIN_SUB, (const char[]{'-', '\0'}), 1);
        case '*': return new_token(KORELIN_MUL, (const char[]){'*', '\0'}, 1);
        case '/': return new_token(KORELIN_DIV, (const char[]){'/', '\0'}, 1);
        case '%': return new_token(KORELIN_MOD, (const char[]){'%', '\0'}, 1);
        case '^': return new_token(KORELIN_POW, (const char[]){'^', '\0'}, 1);
        case '!': return new_token(KORELIN_NOT, (const char[]){'!', '\0'}, 1);
        case '<': return new_token(KORELIN_LT, (const char[]){'<', '\0'}, 1);
        case '>': return new_token(KORELIN_GT, (const char[]){'>', '\0'}, 1);
        case ',': return new_token(KORELIN_COMMA, (const char[]){',', '\0'}, 1);
        case ';': return new_token(KORELIN_SEMICOLON, (const char[]{';', '\0'}), 1);
        case '(': return new_token(KORELIN_LPAREN, (const char[]){'(', '\0'}, 1);
        case ')': return new_token(KORELIN_RPAREN, (const char[]{')', '\0'}), 1);
        case '[': return new_token(KORELIN_LBRACKET, (const char[]){'[', '\0'}, 1);
        case ']': return new_token(KORELIN_RBRACKET, (const char[]{']', '\0'}), 1);
        case '{': return new_token(KORELIN_LBRACE, (const char[]){'{', '\0'}, 1);
        case '}': return new_token(KORELIN_RBRACE, (const char[]){'}', '\0'}, 1);

        // 双字符运算符
        case '&':
            if (peek(lexer) == '&') {
                advance(lexer); // 消耗第二个 '&'
                return new_token(KORELIN_AND, "&&", 2);
            }
            break;
        case '|':
            if (peek(lexer) == '|') {
                advance(lexer); // 消耗第二个 '|'
                return new_token(KORELIN_OR, "||", 2);
            }
            break;
        // 更多双字符运算符可以在这里添加，如 ==, !=, <=, >=, ++, -- 等

        // 文件结束
        case '\0':
            return new_token(KORELIN_EOF, "", 0);

        // 标识符和关键字
        default:
            if (isalpha(lexer->current_char) || lexer->current_char == '_') {
                return read_identifier(lexer);
            }
            // 数字
            else if (isdigit(lexer->current_char)) {
                return read_number(lexer);
            }
            // 无法识别的字符
            else {
                char c = lexer->current_char;
                advance(lexer);
                return new_token(KORELIN_ERROR, (const char[]){c, '\0'}, 1);
            }
    }

    // 默认返回错误
    char c = lexer->current_char;
    advance(lexer);
    return new_token(KORELIN_ERROR, (const char[]){c, '\0'}, 1);
}

// 初始化 Lexer
void init_korelin_lexer(KorelinLexer* lexer, const char* input) {
    lexer->input = input;
    lexer->position = 0;
    lexer->read_position = 0;
    lexer->current_char = '\0';
    advance(lexer); // 读取第一个字符
}

// 释放 Token 的 value 内存
void free_korelin_token(KorelinToken* token) {
    // 只有动态分配的内存才需要释放
    if (token->type == KORELIN_IDENT || token->type == KORELIN_INT || token->type == KORELIN_DOUBLE || token->type == KORELIN_STRING) {
        free((void*)token->value);
        token->value = NULL; // 防止悬挂指针
    }
}