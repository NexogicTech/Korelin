// ast.h
// Created by Helix on 2025/12/31.
//

#ifndef KORELIN_AST_H
#define KORELIN_AST_H

#include "klexer.h"
#include <stdbool.h>
typedef enum {
    // 根节点
    NODE_PROGRAM,

    // 语句 (Statements)
    NODE_LET_STATEMENT,
    NODE_VAR_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_EXPRESSION_STATEMENT,
    NODE_BLOCK_STATEMENT,
    NODE_IF_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_BREAK_STATEMENT,
    NODE_CONTINUE_STATEMENT,

    // 表达式 (Expressions)
    NODE_IDENTIFIER,
    NODE_INTEGER_LITERAL,
    NODE_STRING_LITERAL,
    NODE_BOOLEAN_LITERAL,

    NODE_PREFIX_EXPRESSION,  // e.g., !x, -y
    NODE_INFIX_EXPRESSION,   // e.g., x + y, a == b
    NODE_ASSIGNMENT_EXPRESSION, // e.g., x = 42

    NODE_FUNCTION_LITERAL,   // e.g., func(x, y) { return x + y; }
    NODE_CALL_EXPRESSION,    // e.g., add(1, 2)

    NODE_ARRAY_LITERAL,      // e.g., [1, "two", x]
    NODE_INDEX_EXPRESSION,   // e.g., myArray[0]

    NODE_CLASS_LITERAL,      // e.g., class MyClass { ... }
    NODE_MEMBER_ACCESS_EXPRESSION, // e.g., obj.property, obj.method()

} NodeType;

typedef struct Node {
    NodeType type;
} Node;

// 程序根节点，包含一个语句列表。
typedef struct Program {
    Node node;
    Node** statements;
    size_t statement_count;
} Program;

// let 语句，例如: let x = 42;
typedef struct LetStatement {
    Node node;
    KorelinToken name;         // 标识符的 Token (e.g., Token{KORELIN_IDENT, "x"})
    Node* value;        // 赋值的表达式 (可以为 NULL)
} LetStatement;

// var 语句，例如: let x = 42;
typedef struct VarStatement {
    Node node;
    KorelinToken name;         // 标识符的 Token (e.g., Token{KORELIN_IDENT, "x"})
    Node* value;        // 赋值的表达式 (可以为 NULL)
} VarStatement;

// return 语句，例如: return x + y;
typedef struct ReturnStatement {
    Node node;
    Node* return_value; // 返回的表达式
} ReturnStatement;

// 表达式语句，例如: x + 1;
typedef struct ExpressionStatement {
    Node node;
    Node* expression; // 表达式
} ExpressionStatement;

// 代码块，用 {} 包裹的语句列表。
typedef struct BlockStatement {
    Node node;
    Node** statements;
    size_t statement_count;
} BlockStatement;

// if 语句，支持 elseif 和 else。
typedef struct IfStatement {
    Node node;
    Node* condition;        // if 的条件表达式
    Node* consequence;      // if 为真时执行的 BlockStatement
    Node* alternative;      // if 为假时执行的 BlockStatement (可以为 NULL)
} IfStatement;

// for 语句, e.g., for (let i = 0; i < 10; i++) { ... }
typedef struct ForStatement {
    Node node;
    Node* initializer;  // 初始化语句 (LetStatement 或 ExpressionStatement)
    Node* condition;    // 条件表达式
    Node* update;       // 更新表达式
    Node* body;         // 循环体 (BlockStatement)
} ForStatement;

// while 语句, e.g., while (x > 0) { ... }
typedef struct WhileStatement {
    Node node;
    Node* condition;    // 条件表达式
    Node* body;         // 循环体 (BlockStatement)
} WhileStatement;

// break 语句
typedef struct BreakStatement {
    Node node;
} BreakStatement;

// continue 语句
typedef struct ContinueStatement {
    Node node;
} ContinueStatement;

// 标识符，例如: x, myVariable, add
typedef struct Identifier {
    Node node;
    KorelinToken token; // KORELIN_IDENT 类型的 Token
    const char* value;
} Identifier;

// 整数字面量，例如: 42, 100
typedef struct IntegerLiteral {
    Node node;
    KorelinToken token; // KORELIN_INT 类型的 Token
    long long value; // 使用 long long 以支持更大范围的整数
} IntegerLiteral;

// 字符串字面量，例如: "hello world"
typedef struct StringLiteral {
    Node node;
    KorelinToken token; // KORELIN_STRING 类型的 Token
    const char* value;
} StringLiteral;

// 布尔字面量，例如: true, false
typedef struct BooleanLiteral {
    Node node;
    KorelinToken token; // KORELIN_TRUE 或 KORELIN_FALSE 类型的 Token
    bool value;
} BooleanLiteral;

// 前缀表达式，例如: !isReady, -count
typedef struct PrefixExpression {
    Node node;
    KorelinToken op;   // 运算符 Token, e.g., '!', '-'
    Node* right; // 右边的表达式
} PrefixExpression;

// 中缀表达式，例如: x + y, a == b
typedef struct InfixExpression {
    Node node;
    Node* left; // 左边的表达式
    KorelinToken op;   // 运算符 Token, e.g., '+', '=='
    Node* right; // 右边的表达式
} InfixExpression;

// 赋值表达式，例如: x = 42, obj.prop = value
typedef struct AssignmentExpression {
    Node node;
    Node* left; // 赋值目标 (通常是 Identifier 或 MemberAccess)
    KorelinToken op;   // 赋值运算符, e.g., '='
    Node* right; // 赋值的源
} AssignmentExpression;

// 函数字面量，例如: func(x, y) { return x + y; }
typedef struct FunctionLiteral {
    Node node;
    KorelinToken token;         // 'func' 关键字的 Token
    KorelinToken* parameters;   // 参数列表 (Token 数组)
    size_t param_count;  // 参数数量
    Node* body;          // 函数体 (BlockStatement)
} FunctionLiteral;

// 函数调用表达式，例如: add(1, 2)
typedef struct CallExpression {
    Node node;
    Node* function;      // 被调用的函数 (通常是 Identifier)
    Node** arguments;    // 参数表达式列表
    size_t arg_count;    // 参数数量
} CallExpression;

// 数组字面量，例如: [1, "two", x]
typedef struct ArrayLiteral {
    Node node;
    Node** elements;     // 元素表达式列表
    size_t element_count; // 元素数量
} ArrayLiteral;

// 索引表达式，例如: myArray[0], obj["key"]
typedef struct IndexExpression {
    Node node;
    Node* left;          // 左边的表达式 (数组或对象)
    Node* index;         // 索引表达式
} IndexExpression;

/**
 * @brief 递归地释放 AST 节点及其所有子节点占用的内存。
 * @param node 指向要释放的根节点的指针。
 */
void free_ast(Node* node);

/**
 * @brief 将 AST 节点类型转换为可读的字符串，用于调试。
 * @param type 节点类型。
 * @return 节点类型的字符串表示。
 */
const char* node_type_to_string(NodeType type);

/**
 * @brief 将 AST 以人类可读的格式打印到标准输出，用于调试。
 * @param node 指向要打印的根节点的指针。
 * @param indent_level 缩进级别，用于格式化输出。
 */
void print_ast(Node* node, int indent_level);
#endif //KORELIN_AST_H