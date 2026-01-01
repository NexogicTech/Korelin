//
// Created by Helix on 2025/12/31.
//

#include "ast.h"
#include "klexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Parser 结构体和辅助函数
// =============================================================================

typedef struct {
    KorelinLexer lexer;
    KorelinToken current_token;
    KorelinToken peek_token;
    // 这里可以添加一个错误数组，用于收集解析过程中的错误
} KorelinParser;

// 初始化 Parser
void init_parser(KorelinParser* parser, const char* input) {
    init_korelin_lexer(&parser->lexer, input);
    // 读取两个 Token 来初始化 current 和 peek
    parser->current_token = next_korelin_token(&parser->lexer);
    parser->peek_token = next_korelin_token(&parser->lexer);
}

// 兼容性辅助函数：strndup
static char* k_strndup(const char* s, size_t n) {
    size_t len = 0;
    while (len < n && s[len] != '\0') {
        len++;
    }
    char* new_str = malloc(len + 1);
    if (!new_str) return NULL;
    memcpy(new_str, s, len);
    new_str[len] = '\0';
    return new_str;
}

// 辅助函数：将 Token 复制到 AST 节点中 (如果是动态分配的，则深拷贝)
static void copy_token_to_ast(KorelinToken* dest, const KorelinToken* src) {
    *dest = *src;
    if (src->needs_free) {
        dest->value = k_strndup(src->value, src->length);
        dest->needs_free = 1;
    } else {
        dest->needs_free = 0;
    }
}

// 辅助函数：检查当前 Token 是否为指定类型
static bool current_token_is(const KorelinParser* parser, KorelinTokenType type) {
    return parser->current_token.type == type;
}

// 辅助函数：检查下一个 Token 是否为指定类型
static bool peek_token_is(const KorelinParser* parser, KorelinTokenType type) {
    return parser->peek_token.type == type;
}

// 辅助函数：如果下一个 Token 是指定类型，则消耗它并返回 true，否则返回 false
static bool expect_peek(KorelinParser* parser, KorelinTokenType type) {
    if (peek_token_is(parser, type)) {
        // 前进一个 Token
        parser->current_token = parser->peek_token;
        parser->peek_token = next_korelin_token(&parser->lexer);
        return true;
    }
    fprintf(stderr, "Expected next token to be %d, got %d instead.\n", type, parser->peek_token.type);
    return false;
}

// 前向声明
static Node* parse_statement(KorelinParser* parser);
static void next_token(KorelinParser* parser); // 添加 next_token 前向声明

// 错误恢复：同步解析器状态
static void synchronize(KorelinParser* parser) {
    next_token(parser);

    while (parser->current_token.type != KORELIN_EOF) {
        if (parser->current_token.type == KORELIN_SEMICOLON) return;

        switch (parser->peek_token.type) {
            case KORELIN_CLASS:
            case KORELIN_FUNC:
            case KORELIN_VAR:
            case KORELIN_LET:
            case KORELIN_FOR:
            case KORELIN_IF:
            case KORELIN_WHILE:
            case KORELIN_RETURN:
                return;
            default:
                // Do nothing
                break;
        }

        next_token(parser);
    }
}

// 解析代码块
static Node* parse_block_statement(KorelinParser* parser) {
    BlockStatement* block = malloc(sizeof(BlockStatement));
    block->node.type = NODE_BLOCK_STATEMENT;
    block->statements = NULL;
    block->statement_count = 0;

    next_token(parser); // 跳过 '{'

    while (!current_token_is(parser, KORELIN_RBRACE) && !current_token_is(parser, KORELIN_EOF)) {
        Node* stmt = parse_statement(parser);
        if (stmt != NULL) {
            size_t new_size = block->statement_count + 1;
            Node** new_stmts = realloc(block->statements, new_size * sizeof(Node*));
            if (new_stmts) {
                block->statements = new_stmts;
                block->statements[block->statement_count++] = stmt;
            } else {
                // Handle allocation error
                free(stmt); // Avoid leak
            }
        }
        next_token(parser);
    }

    return (Node*)block;
}


// 辅助函数：消耗当前 Token，前进一个
static void next_token(KorelinParser* parser) {
    free_korelin_token(&parser->current_token); // 释放旧 Token 的内存
    parser->current_token = parser->peek_token;
    parser->peek_token = next_korelin_token(&parser->lexer);
}

// =============================================================================
// 表达式解析函数 (遵循 Pratt Parsing 思想处理优先级)
// =============================================================================

// 表达式优先级，数字越大优先级越高
typedef enum {
    PREC_LOWEST,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // ||
    PREC_AND,         // &&
    PREC_EQUALS,      // == !=
    PREC_COMPARISON,  // > < >= <=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / %
    PREC_UNARY,       // ! -
    PREC_CALL,        // myFunction(x)
    PREC_INDEX        // array[index]
} Precedence;

// 获取给定 Token 类型的优先级
static Precedence peek_precedence(const KorelinParser* parser) {
    switch (parser->peek_token.type) {
        case KORELIN_ASSIGN: return PREC_ASSIGNMENT;
        case KORELIN_OR: return PREC_OR;
        case KORELIN_AND: return PREC_AND;
        case KORELIN_EQ: case KORELIN_NOT_EQ: return PREC_EQUALS;
        case KORELIN_LT: case KORELIN_GT: case KORELIN_LE: case KORELIN_GE: return PREC_COMPARISON;
        case KORELIN_ADD: case KORELIN_SUB: return PREC_TERM;
        case KORELIN_MUL: case KORELIN_DIV: case KORELIN_MOD: return PREC_FACTOR;
        case KORELIN_LPAREN: return PREC_CALL;
        case KORELIN_LBRACKET: return PREC_INDEX;
        default: return PREC_LOWEST;
    }
}

static Precedence current_precedence(const KorelinParser* parser) {
    switch (parser->current_token.type) {
        case KORELIN_ASSIGN: return PREC_ASSIGNMENT;
        case KORELIN_OR: return PREC_OR;
        case KORELIN_AND: return PREC_AND;
        case KORELIN_EQ: case KORELIN_NOT_EQ: return PREC_EQUALS;
        case KORELIN_LT: case KORELIN_GT: case KORELIN_LE: case KORELIN_GE: return PREC_COMPARISON;
        case KORELIN_ADD: case KORELIN_SUB: return PREC_TERM;
        case KORELIN_MUL: case KORELIN_DIV: case KORELIN_MOD: return PREC_FACTOR;
        case KORELIN_LPAREN: return PREC_CALL;
        case KORELIN_LBRACKET: return PREC_INDEX;
        default: return PREC_LOWEST;
    }
}

// 解析主表达式
static Node* parse_expression(KorelinParser* parser, Precedence precedence);

// 解析分组表达式 (e.g., (x + y))
static Node* parse_grouped_expression(KorelinParser* parser) {
    next_token(parser); // 跳过 '('
    Node* expr = parse_expression(parser, PREC_LOWEST);
    if (!expect_peek(parser, KORELIN_RPAREN)) {
        return NULL;
    }
    return expr;
}

// 解析基本表达式 (字面量、标识符、分组表达式)
static Node* parse_primary(KorelinParser* parser) {
    switch (parser->current_token.type) {
        case KORELIN_INT: {
            IntegerLiteral* lit = malloc(sizeof(IntegerLiteral));
            lit->node.type = NODE_INTEGER_LITERAL;
            copy_token_to_ast(&lit->token, &parser->current_token);
            lit->value = atoll(parser->current_token.value);
            return (Node*)lit;
        }
        case KORELIN_STRING: {
            StringLiteral* lit = malloc(sizeof(StringLiteral));
            lit->node.type = NODE_STRING_LITERAL;
            copy_token_to_ast(&lit->token, &parser->current_token);
            // 移除首尾的引号
            if (parser->current_token.length >= 2) {
                lit->value = k_strndup(parser->current_token.value + 1, parser->current_token.length - 2);
            } else {
                lit->value = k_strndup("", 0);
            }
            return (Node*)lit;
        }
        case KORELIN_TRUE: case KORELIN_FALSE: {
            BooleanLiteral* lit = malloc(sizeof(BooleanLiteral));
            lit->node.type = NODE_BOOLEAN_LITERAL;
            copy_token_to_ast(&lit->token, &parser->current_token);
            lit->value = (parser->current_token.type == KORELIN_TRUE);
            return (Node*)lit;
        }
        case KORELIN_IDENT: {
            Identifier* ident = malloc(sizeof(Identifier));
            ident->node.type = NODE_IDENTIFIER;
            copy_token_to_ast(&ident->token, &parser->current_token);
            ident->value = k_strndup(parser->current_token.value, parser->current_token.length);
            return (Node*)ident;
        }
        case KORELIN_LPAREN:
            return parse_grouped_expression(parser);
        // ... 其他 primary，如 func, class, [ ... ]
        default:
            fprintf(stderr, "Unexpected token %d in primary expression.\n", parser->current_token.type);
            return NULL;
    }
}

// 解析前缀表达式 (e.g., !x, -y)
static Node* parse_prefix_expression(KorelinParser* parser) {
    PrefixExpression* expr = malloc(sizeof(PrefixExpression));
    expr->node.type = NODE_PREFIX_EXPRESSION;
    expr->op = parser->current_token;
    next_token(parser); // 消耗前缀运算符
    expr->right = parse_expression(parser, PREC_UNARY);
    return (Node*)expr;
}

// 解析中缀表达式 (e.g., x + y)
static Node* parse_infix_expression(KorelinParser* parser, Node* left) {
    InfixExpression* expr = malloc(sizeof(InfixExpression));
    expr->node.type = NODE_INFIX_EXPRESSION;
    expr->left = left;
    expr->op = parser->current_token;
    Precedence precedence = current_precedence(parser);
    next_token(parser); // 消耗中缀运算符
    expr->right = parse_expression(parser, precedence);
    return (Node*)expr;
}

// 解析赋值表达式 (e.g., x = 42)
static Node* parse_assignment_expression(KorelinParser* parser, Node* left) {
    AssignmentExpression* expr = malloc(sizeof(AssignmentExpression));
    expr->node.type = NODE_ASSIGNMENT_EXPRESSION;
    expr->left = left;
    copy_token_to_ast(&expr->op, &parser->current_token);
    next_token(parser); // 消耗 '='
    // 使用 PREC_LOWEST 以支持右结合性 (e.g. a = b = c)
    expr->right = parse_expression(parser, PREC_LOWEST);
    return (Node*)expr;
}


// 主表达式解析循环
static Node* parse_expression(KorelinParser* parser, Precedence precedence) {
    Node* left = NULL;

    // 1. 解析前缀部分
    switch (parser->current_token.type) {
        case KORELIN_NOT: case KORELIN_SUB:
            left = parse_prefix_expression(parser);
            break;
        default:
            left = parse_primary(parser);
            break;
    }

    if (!left) return NULL;

    // 2. 循环解析中缀部分
    while (!peek_token_is(parser, KORELIN_SEMICOLON) && precedence < peek_precedence(parser)) {
        switch (parser->peek_token.type) {
            case KORELIN_ADD: case KORELIN_SUB: case KORELIN_MUL: case KORELIN_DIV: case KORELIN_MOD:
            case KORELIN_EQ: case KORELIN_NOT_EQ: case KORELIN_LT: case KORELIN_GT: case KORELIN_LE: case KORELIN_GE:
            case KORELIN_AND: case KORELIN_OR:
                next_token(parser); // 前进到中缀运算符
                left = parse_infix_expression(parser, left);
                break;
            case KORELIN_ASSIGN:
                next_token(parser); // 前进到 '='
                left = parse_assignment_expression(parser, left);
                break;
            // ... 其他中缀或后缀表达式，如函数调用、数组索引
            default:
                return left; // 没有更多中缀表达式了
        }
    }

    return left;
}


// =============================================================================
// 语句解析函数
// =============================================================================

// 解析 let 语句
static Node* parse_let_statement(KorelinParser* parser) {
    LetStatement* stmt = malloc(sizeof(LetStatement));
    stmt->node.type = NODE_LET_STATEMENT;
    // stmt->name = parser->current_token; // 'let' token (ignored/overwritten)

    if (!expect_peek(parser, KORELIN_IDENT)) {
        free(stmt);
        return NULL;
    }
    copy_token_to_ast(&stmt->name, &parser->current_token); // identifier token

    if (expect_peek(parser, KORELIN_ASSIGN)) {
        next_token(parser); // 跳过 '='
        stmt->value = parse_expression(parser, PREC_LOWEST);
    } else {
        stmt->value = NULL;
    }

    while (!current_token_is(parser, KORELIN_SEMICOLON) && !current_token_is(parser, KORELIN_EOF)) {
        next_token(parser);
    }

    return (Node*)stmt;
}

// 解析 var 语句 (与 let 类似)
static Node* parse_var_statement(KorelinParser* parser) {
    VarStatement* stmt = malloc(sizeof(VarStatement));
    stmt->node.type = NODE_VAR_STATEMENT;
    // stmt->name = parser->current_token; // 'var' token (ignored/overwritten)

    if (!expect_peek(parser, KORELIN_IDENT)) {
        free(stmt);
        return NULL;
    }
    copy_token_to_ast(&stmt->name, &parser->current_token); // identifier token

    if (expect_peek(parser, KORELIN_ASSIGN)) {
        next_token(parser); // 跳过 '='
        stmt->value = parse_expression(parser, PREC_LOWEST);
    } else {
        stmt->value = NULL;
    }

    while (!current_token_is(parser, KORELIN_SEMICOLON) && !current_token_is(parser, KORELIN_EOF)) {
        next_token(parser);
    }

    return (Node*)stmt;
}


// 解析 return 语句
static Node* parse_return_statement(KorelinParser* parser) {
    ReturnStatement* stmt = malloc(sizeof(ReturnStatement));
    stmt->node.type = NODE_RETURN_STATEMENT;
    next_token(parser); // 跳过 'return'
    stmt->return_value = parse_expression(parser, PREC_LOWEST);
    while (!current_token_is(parser, KORELIN_SEMICOLON) && !current_token_is(parser, KORELIN_EOF)) {
        next_token(parser);
    }
    return (Node*)stmt;
}

// 解析表达式语句
static Node* parse_expression_statement(KorelinParser* parser) {
    ExpressionStatement* stmt = malloc(sizeof(ExpressionStatement));
    stmt->node.type = NODE_EXPRESSION_STATEMENT;
    stmt->expression = parse_expression(parser, PREC_LOWEST);

    if (stmt->expression == NULL) {
        free(stmt);
        return NULL;
    }

    if (peek_token_is(parser, KORELIN_SEMICOLON)) {
        next_token(parser); // 跳过 ';'
    }
    return (Node*)stmt;
}

// 解析 if 语句
static Node* parse_if_statement(KorelinParser* parser) {
    IfStatement* stmt = malloc(sizeof(IfStatement));
    stmt->node.type = NODE_IF_STATEMENT;

    if (!expect_peek(parser, KORELIN_LPAREN)) {
        free(stmt);
        return NULL;
    }
    next_token(parser); // 跳过 '('
    stmt->condition = parse_expression(parser, PREC_LOWEST);
    if (!expect_peek(parser, KORELIN_RPAREN)) {
        free_ast((Node*)stmt);
        return NULL;
    }
    if (!expect_peek(parser, KORELIN_LBRACE)) {
        free_ast((Node*)stmt);
        return NULL;
    }
    // 注意：这里需要一个 parse_block_statement 函数
    // 为简化，我们假设它存在并返回一个 BlockStatement
    stmt->consequence = parse_block_statement(parser);

    if (peek_token_is(parser, KORELIN_ELSE)) {
        next_token(parser); // 跳过 'else'
        if (peek_token_is(parser, KORELIN_IF)) {
            next_token(parser); // 跳过 'if'
            // stmt->alternative = parse_if_statement(parser); // 支持 else if
            // 注意：IfStatement 结构体的 alternative 是 Node* 类型，可以直接赋值
            // 但这里为了简化，我们暂时只处理 block
            // 实际上递归调用 parse_if_statement 是正确的做法，但需要类型转换
            stmt->alternative = parse_if_statement(parser);
        } else if (expect_peek(parser, KORELIN_LBRACE)) {
            stmt->alternative = parse_block_statement(parser);
        }
    }
    return (Node*)stmt;
}


// 分发解析单个语句
static Node* parse_statement(KorelinParser* parser) {
    switch (parser->current_token.type) {
        case KORELIN_LET:
            return parse_let_statement(parser);
        case KORELIN_VAR:
            return parse_var_statement(parser);
        case KORELIN_RETURN:
            return parse_return_statement(parser);
        case KORELIN_IF:
            return parse_if_statement(parser);
        // ... 其他语句类型
        default:
            return parse_expression_statement(parser);
    }
}


// =============================================================================
// 入口函数
// =============================================================================

/**
 * @brief 解析输入的源代码并返回一个 AST 的根节点 (Program)。
 * @param input 源代码字符串。
 * @return 指向 Program 节点的指针。调用者需要负责调用 free_ast 释放内存。
 */
Program* parse_program(const char* input) {
    KorelinParser parser;
    init_parser(&parser, input);

    Program* program = malloc(sizeof(Program));
    program->node.type = NODE_PROGRAM;
    program->statements = NULL;
    program->statement_count = 0;

    while (!current_token_is(&parser, KORELIN_EOF)) {
        // 忽略空语句 (单独的分号)
        if (current_token_is(&parser, KORELIN_SEMICOLON)) {
            next_token(&parser);
            continue;
        }

        Node* stmt = parse_statement(&parser);
        if (stmt != NULL) {
            // 动态扩展 statements 数组
            size_t new_size = program->statement_count + 1;
            Node** new_stmts = realloc(program->statements, new_size * sizeof(Node*));
            if (!new_stmts) {
                fprintf(stderr, "Failed to realloc in parse_program\n");
                // 这里应该进行错误处理和内存清理
                free_ast((Node*)program);
                free_korelin_token(&parser.current_token);
                free_korelin_token(&parser.peek_token);
                return NULL;
            }
            program->statements = new_stmts;
            program->statements[program->statement_count++] = stmt;
        } else {
            // 发生解析错误，进行错误恢复
            synchronize(&parser);
            continue; 
        }
        next_token(&parser); // 前进到下一个 Token
    }

    // 清理 Parser 中的剩余 Token
    free_korelin_token(&parser.current_token);
    free_korelin_token(&parser.peek_token);

    return program;
}