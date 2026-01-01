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
static char peek(const KorelinLexer* lexer) {
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
    return (KorelinToken){.type = type, .value = value, .length = length, .needs_free = 0};
}

// 辅助函数：读取字符串字面量
static KorelinToken read_string(KorelinLexer* lexer) {
    size_t start_pos = lexer->position;
    char quote_char = lexer->current_char;
    advance(lexer); // 跳过起始引号

    while (lexer->current_char != quote_char && lexer->current_char != '\0') {
        if (lexer->current_char == '\\' && peek(lexer) == quote_char) {
            advance(lexer); // 跳过转义字符
        }
        advance(lexer);
    }

    if (lexer->current_char == quote_char) {
        advance(lexer); // 消耗结束引号
    }

    size_t len = lexer->position - start_pos;
    char* literal = malloc(len + 1);
    if (!literal) {
        fprintf(stderr, "Error: malloc failed in read_string\n");
        exit(EXIT_FAILURE);
    }
    strncpy(literal, lexer->input + start_pos, len);
    literal[len] = '\0';

    return (KorelinToken){.type = KORELIN_STRING, .value = literal, .length = len, .needs_free = 1};
}

// 关键字查找函数
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
    if (strcmp(ident, "int") == 0) return KORELIN_TYPE_INT32;
    if (strcmp(ident, "long") == 0) return KORELIN_TYPE_LONG64;
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

    KorelinTokenType type = lookup_ident(literal);
    KorelinToken token = {.type = type, .value = literal, .length = len, .needs_free = 1};
    return token;
}

// 辅助函数：读取一个完整的数字 (当前只支持整数)
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

    KorelinToken token = {.type = KORELIN_INT, .value = literal, .length = len, .needs_free = 1};
    return token;
}

// 核心函数：获取下一个 Token (已优化)
KorelinToken next_korelin_token(KorelinLexer* lexer) {
    skip_whitespace(lexer);

    KorelinToken token;

    switch (lexer->current_char) {
        // --- 双字符运算符 (必须在单字符之前检查) ---
        case '=':
            if (peek(lexer) == '=') {
                token = new_token(KORELIN_EQ, "==", 2);
                advance(lexer); // 消耗第二个 '='
            } else {
                token = new_token(KORELIN_ASSIGN, "=", 1);
            }
            break;
        case '+':
            if (peek(lexer) == '+') {
                token = new_token(KORELIN_INCREMENT, "++", 2);
                advance(lexer);
            } else if (peek(lexer) == '=') {
                token = new_token(KORELIN_ADD_ASSIGN, "+=", 2);
                advance(lexer);
            } else {
                token = new_token(KORELIN_ADD, "+", 1);
            }
            break;
        case '-':
            if (peek(lexer) == '-') {
                token = new_token(KORELIN_DECREMENT, "--", 2);
                advance(lexer);
            } else if (peek(lexer) == '=') {
                token = new_token(KORELIN_SUB_ASSIGN, "-=", 2);
                advance(lexer);
            } else {
                token = new_token(KORELIN_SUB, "-", 1);
            }
            break;
        case '!':
            if (peek(lexer) == '=') {
                token = new_token(KORELIN_NOT_EQ, "!=", 2);
                advance(lexer);
            } else {
                token = new_token(KORELIN_NOT, "!", 1);
            }
            break;
        case '<':
            if (peek(lexer) == '=') {
                token = new_token(KORELIN_LE, "<=", 2);
                advance(lexer);
            } else {
                token = new_token(KORELIN_LT, "<", 1);
            }
            break;
        case '>':
            if (peek(lexer) == '=') {
                token = new_token(KORELIN_GE, ">=", 2);
                advance(lexer);
            } else {
                token = new_token(KORELIN_GT, ">", 1);
            }
            break;
        case '&':
            if (peek(lexer) == '&') {
                token = new_token(KORELIN_AND, "&&", 2);
                advance(lexer);
            } else if (peek(lexer) == '=') {
                token = new_token(KORELIN_BIT_AND_ASSIGN, "&=", 2);
                advance(lexer);
            } else {
                token = new_token(KORELIN_BIT_AND, "&", 1); // 正确处理单 '&'
            }
            break;
        case '|':
            if (peek(lexer) == '|') {
                token = new_token(KORELIN_OR, "||", 2);
                advance(lexer);
            } else if (peek(lexer) == '=') {
                token = new_token(KORELIN_BIT_OR_ASSIGN, "|=", 2);
                advance(lexer);
            } else {
                token = new_token(KORELIN_BIT_OR, "|", 1); // 正确处理单 '|'
            }
            break;
        // ... 在这里可以继续添加其他双字符运算符的处理，如 *=, /= 等

        // --- 单字符运算符和分隔符 ---
        case '*': token = new_token(KORELIN_MUL, "*", 1); break;
        case '/': token = new_token(KORELIN_DIV, "/", 1); break;
        case '%': token = new_token(KORELIN_MOD, "%", 1); break;
        case '^': token = new_token(KORELIN_POW, "^", 1); break;
        case ',': token = new_token(KORELIN_COMMA, ",", 1); break;
        case ';': token = new_token(KORELIN_SEMICOLON, ";", 1); break;
        case '(': token = new_token(KORELIN_LPAREN, "(", 1); break;
        case ')': token = new_token(KORELIN_RPAREN, ")", 1); break;
        case '[': token = new_token(KORELIN_LBRACKET, "[", 1); break;
        case ']': token = new_token(KORELIN_RBRACKET, "]", 1); break;
        case '{': token = new_token(KORELIN_LBRACE, "{", 1); break;
        case '}': token = new_token(KORELIN_RBRACE, "}", 1); break;

        // --- 文件结束 ---
        case '\0':
            token = new_token(KORELIN_EOF, "", 0);
            break;

        // --- 字符串字面量 ---
        case '"':
        case '\'':
            return read_string(lexer);

        // --- 默认情况：标识符、数字或错误 ---
        default:
            if (isalpha(lexer->current_char) || lexer->current_char == '_') {
                return read_identifier(lexer); // 直接返回，无需 advance
            } else if (isdigit(lexer->current_char)) {
                return read_number(lexer); // 直接返回，无需 advance
            } else {
                // 无法识别的字符
                token = new_token(KORELIN_ERROR, (const char[]){lexer->current_char, '\0'}, 1);
            }
            break;
    }

    advance(lexer); // 消耗当前字符
    return token;
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
    if (token->type == KORELIN_IDENT || token->type == KORELIN_INT ||
        token->type == KORELIN_DOUBLE || token->type == KORELIN_STRING) {
        free((void*)token->value);
        token->value = NULL; // 防止悬挂指针
    }
}