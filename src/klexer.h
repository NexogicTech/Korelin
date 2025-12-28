//
// Created by Helix on 2025/12/28.
//

#ifndef KORELIN_LEXER_H
#define KORELIN_LEXER_H

#include <stddef.h> // 为了 size_t

// 定义Token类型
typedef enum {
    // 特殊
    KORELIN_EOF,
    KORELIN_ERROR,

    // 标识符和字面量
    KORELIN_IDENT,          // 变量名, 如: x, answer
    KORELIN_INT,            // 整数, 如: 12345, 0, 1
    KORELIN_DOUBLE,         // 浮点数, 如: 12.21
    KORELIN_STRING,         // 字符串, 如: "abcd" or 'abcd'

    // 分隔符
    KORELIN_COMMA,          // ,
    KORELIN_SEMICOLON,      // ;
    KORELIN_LPAREN,         // (
    KORELIN_RPAREN,         // )
    KORELIN_LBRACKET,       // [
    KORELIN_RBRACKET,       // ]
    KORELIN_LBRACE,         // {
    KORELIN_RBRACE,         // }

    // 运算符
    // 单字符
    KORELIN_ASSIGN,         // =
    KORELIN_ADD,            // +
    KORELIN_SUB,            // -
    KORELIN_MUL,            // *
    KORELIN_DIV,            // /
    KORELIN_MOD,            // %
    KORELIN_POW,            // ^
    KORELIN_NOT,            // !
    KORELIN_LT,             // <
    KORELIN_GT,             // >
    KORELIN_BIT_AND,        // &  (新增)
    KORELIN_BIT_OR,         // |  (新增)
    KORELIN_BIT_XOR,        // ^  (如果需要)
    KORELIN_BIT_NOT,        // ~  (如果需要)
    KORELIN_SHL,            // << (如果需要)
    KORELIN_SHR,            // >> (如果需要)

    // 双字符
    KORELIN_INCREMENT,      // ++
    KORELIN_DECREMENT,      // --
    KORELIN_EQ,             // ==
    KORELIN_NOT_EQ,         // !=
    KORELIN_LE,             // <=
    KORELIN_GE,             // >=
    KORELIN_AND,            // &&
    KORELIN_OR,             // ||
    KORELIN_ADD_ASSIGN,     // +=
    KORELIN_SUB_ASSIGN,     // -=
    KORELIN_MUL_ASSIGN,     // *=
    KORELIN_DIV_ASSIGN,     // /=
    KORELIN_MOD_ASSIGN,     // %=
    KORELIN_POW_ASSIGN,     // ^=
    KORELIN_BIT_AND_ASSIGN, // &= (新增)
    KORELIN_BIT_OR_ASSIGN,  // |= (新增)
    // ... 其他复合赋值

    // 关键字
    KORELIN_IMPORT,         // import
    KORELIN_STRUCT,         // struct
    KORELIN_VAR,            // var
    KORELIN_LET,            // let
    KORELIN_CONST,          // const
    KORELIN_FUNC,           // func
    KORELIN_RETURN,         // return
    KORELIN_BREAK,          // break
    KORELIN_CONTINUE,       // continue
    KORELIN_CLASS,          // class
    KORELIN_STATIC,         // static
    KORELIN_PUBLIC,         // public
    KORELIN_PROTECTED,      // protected
    KORELIN_PRIVATE,        // private
    KORELIN_IF,             // if
    KORELIN_ELSE,           // else
    KORELIN_ELSEIF,         // elseif
    KORELIN_TRUE,           // true (关键字)
    KORELIN_FALSE,          // false (关键字)
    KORELIN_FOR,            // for
    KORELIN_WHILE,          // while

    // 类型关键字
    KORELIN_TYPE_INT32,     // int
    KORELIN_TYPE_LONG64,    // long
    KORELIN_TYPE_DOUBLE,    // double
    KORELIN_TYPE_STRING,    // string
    KORELIN_TYPE_BOOL,      // bool

} KorelinTokenType;

// Token结构体
typedef struct {
    KorelinTokenType type;
    const char* value;      // 指向动态分配的Token文本值的指针，需要手动释放
    size_t length;          // 值的长度
} KorelinToken;

// 词法分析器 (Lexer) 结构体
typedef struct {
    const char* input;
    size_t position;        // 当前正在检查的字符的索引
    size_t read_position;   // 下一个要检查的字符的索引
    char current_char;      // 当前正在检查的字符
} KorelinLexer;

// --- 函数声明 ---

/**
 * @brief 初始化一个新的 KorelinLexer。
 * @param lexer 指向要初始化的 KorelinLexer 结构体的指针。
 * @param input 要进行词法分析的源字符串。
 */
void init_korelin_lexer(KorelinLexer* lexer, const char* input);

/**
 * @brief 从输入中读取并返回下一个 Token。
 * @param lexer 指向 KorelinLexer 结构体的指针。
 * @return 一个 KorelinToken 结构体。如果 Token 的 value 是动态分配的，
 *         调用者有责任在使用完毕后调用 free_korelin_token 来释放内存。
 */
KorelinToken next_korelin_token(KorelinLexer* lexer);

/**
 * @brief 释放 KorelinToken 中动态分配的 value 内存。
 * @param token 指向要释放内存的 KorelinToken 结构体的指针。
 */
void free_korelin_token(KorelinToken* token);

/**
 * @brief 根据标识符字符串查找其对应的 Token 类型（关键字或普通标识符）。
 * @param ident 要查找的标识符字符串。
 * @return 对应的 KorelinTokenType。
 */
KorelinTokenType lookup_ident(const char* ident);

#endif // KORELIN_LEXER_H