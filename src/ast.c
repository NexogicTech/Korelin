// ast.c
// Created by Helix on 2025/12/31.
//

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

// 辅助函数：打印缩进
static void print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

/**
 * @brief 将 AST 节点类型转换为可读的字符串，用于调试。
 */
const char* node_type_to_string(NodeType type) {
    switch (type) {
        case NODE_PROGRAM: return "Program";
        case NODE_LET_STATEMENT: return "LetStatement";
        case NODE_VAR_STATEMENT: return "VarStatement";
        case NODE_RETURN_STATEMENT: return "ReturnStatement";
        case NODE_EXPRESSION_STATEMENT: return "ExpressionStatement";
        case NODE_BLOCK_STATEMENT: return "BlockStatement";
        case NODE_IF_STATEMENT: return "IfStatement";
        case NODE_FOR_STATEMENT: return "ForStatement";
        case NODE_WHILE_STATEMENT: return "WhileStatement";
        case NODE_BREAK_STATEMENT: return "BreakStatement";
        case NODE_CONTINUE_STATEMENT: return "ContinueStatement";
        case NODE_IDENTIFIER: return "Identifier";
        case NODE_INTEGER_LITERAL: return "IntegerLiteral";
        case NODE_STRING_LITERAL: return "StringLiteral";
        case NODE_BOOLEAN_LITERAL: return "BooleanLiteral";
        case NODE_PREFIX_EXPRESSION: return "PrefixExpression";
        case NODE_INFIX_EXPRESSION: return "InfixExpression";
        case NODE_ASSIGNMENT_EXPRESSION: return "AssignmentExpression";
        case NODE_FUNCTION_LITERAL: return "FunctionLiteral";
        case NODE_CALL_EXPRESSION: return "CallExpression";
        case NODE_ARRAY_LITERAL: return "ArrayLiteral";
        case NODE_INDEX_EXPRESSION: return "IndexExpression";
        case NODE_CLASS_LITERAL: return "ClassLiteral";
        case NODE_MEMBER_ACCESS_EXPRESSION: return "MemberAccessExpression";
        // ... 其他节点
        default: return "UnknownNode";
    }
}

/**
 * @brief 递归地将 AST 以人类可读的格式打印到标准输出。
 */
void print_ast(Node* node, int indent_level) {
    if (!node) return;

    print_indent(indent_level);
    printf("%s", node_type_to_string(node->type));

    switch (node->type) {
        case NODE_IDENTIFIER: {
            Identifier* ident = (Identifier*)node;
            printf(" (value: '%s')\n", ident->value);
            break;
        }
        case NODE_INTEGER_LITERAL: {
            IntegerLiteral* lit = (IntegerLiteral*)node;
            printf(" (value: %lld)\n", lit->value);
            break;
        }
        case NODE_STRING_LITERAL: {
            StringLiteral* lit = (StringLiteral*)node;
            printf(" (value: \"%s\")\n", lit->value);
            break;
        }
        case NODE_BOOLEAN_LITERAL: {
            BooleanLiteral* lit = (BooleanLiteral*)node;
            printf(" (value: %s)\n", lit->value ? "true" : "false");
            break;
        }
        case NODE_PREFIX_EXPRESSION: {
            PrefixExpression* expr = (PrefixExpression*)node;
            printf(" (operator: '%.*s')\n", (int)expr->op.length, expr->op.value);
            print_ast(expr->right, indent_level + 1);
            break;
        }
        case NODE_INFIX_EXPRESSION: {
            InfixExpression* expr = (InfixExpression*)node;
            printf(" (operator: '%.*s')\n", (int)expr->op.length, expr->op.value);
            print_ast(expr->left, indent_level + 1);
            print_ast(expr->right, indent_level + 1);
            break;
        }
        case NODE_ASSIGNMENT_EXPRESSION: {
            AssignmentExpression* expr = (AssignmentExpression*)node;
            printf(" (operator: '%.*s')\n", (int)expr->op.length, expr->op.value);
            print_ast(expr->left, indent_level + 1);
            print_ast(expr->right, indent_level + 1);
            break;
        }
        case NODE_LET_STATEMENT: {
            LetStatement* stmt = (LetStatement*)node;
            printf(" (name: '%.*s')\n", (int)stmt->name.length, stmt->name.value);
            print_ast(stmt->value, indent_level + 1);
            break;
        }
        case NODE_VAR_STATEMENT: {
            VarStatement* stmt = (VarStatement*)node;
            printf(" (name: '%.*s')\n", (int)stmt->name.length, stmt->name.value);
            print_ast(stmt->value, indent_level + 1);
            break;
        }
        case NODE_RETURN_STATEMENT: {
            ReturnStatement* stmt = (ReturnStatement*)node;
            printf("\n");
            print_ast(stmt->return_value, indent_level + 1);
            break;
        }
        case NODE_EXPRESSION_STATEMENT: {
            ExpressionStatement* stmt = (ExpressionStatement*)node;
            printf("\n");
            print_ast(stmt->expression, indent_level + 1);
            break;
        }
        case NODE_BLOCK_STATEMENT: {
            BlockStatement* block = (BlockStatement*)node;
            printf("\n");
            for (size_t i = 0; i < block->statement_count; i++) {
                print_ast(block->statements[i], indent_level + 1);
            }
            break;
        }
        case NODE_IF_STATEMENT: {
            IfStatement* stmt = (IfStatement*)node;
            printf("\n");
            print_indent(indent_level + 1); printf("Condition:\n");
            print_ast(stmt->condition, indent_level + 2);
            print_indent(indent_level + 1); printf("Consequence:\n");
            print_ast(stmt->consequence, indent_level + 2);
            if (stmt->alternative) {
                print_indent(indent_level + 1); printf("Alternative:\n");
                print_ast(stmt->alternative, indent_level + 2);
            }
            break;
        }
        case NODE_PROGRAM: {
            Program* program = (Program*)node;
            printf("\n");
            for (size_t i = 0; i < program->statement_count; i++) {
                print_ast(program->statements[i], indent_level + 1);
            }
            break;
        }
        // ... 为其他节点类型添加 case
        default:
            printf("\n");
            break;
    }
}

/**
 * @brief 递归地释放 AST 节点及其所有子节点占用的内存。
 */
void free_ast(Node* node) {
    if (!node) return;

    switch (node->type) {
        case NODE_IDENTIFIER: {
            Identifier* ident = (Identifier*)node;
            free((void*)ident->value); // 释放字符串
            free_korelin_token(&ident->token); // 释放 Token
            free(ident);
            break;
        }
        case NODE_INTEGER_LITERAL: {
            IntegerLiteral* lit = (IntegerLiteral*)node;
            free_korelin_token(&lit->token); // 释放 Token 的 value
            free(lit);
            break;
        }
        case NODE_STRING_LITERAL: {
            StringLiteral* lit = (StringLiteral*)node;
            free((void*)lit->value); // 释放字符串内容
            free_korelin_token(&lit->token); // 释放 Token 的 value
            free(lit);
            break;
        }
        case NODE_BOOLEAN_LITERAL: {
            BooleanLiteral* lit = (BooleanLiteral*)node;
            free_korelin_token(&lit->token); // 释放 Token 的 value
            free(lit);
            break;
        }
        case NODE_PREFIX_EXPRESSION: {
            PrefixExpression* expr = (PrefixExpression*)node;
            free_ast(expr->right);
            free_korelin_token(&expr->op); // 释放运算符 Token 的 value
            free(expr);
            break;
        }
        case NODE_INFIX_EXPRESSION: {
            InfixExpression* expr = (InfixExpression*)node;
            free_ast(expr->left);
            free_ast(expr->right);
            free_korelin_token(&expr->op); // 释放运算符 Token 的 value
            free(expr);
            break;
        }
        case NODE_ASSIGNMENT_EXPRESSION: {
            AssignmentExpression* expr = (AssignmentExpression*)node;
            free_ast(expr->left);
            free_ast(expr->right);
            free_korelin_token(&expr->op);
            free(expr);
            break;
        }
        case NODE_LET_STATEMENT: {
            LetStatement* stmt = (LetStatement*)node;
            free_ast(stmt->value);
            free_korelin_token(&stmt->name); // 释放 name Token
            free(stmt);
            break;
        }
        case NODE_VAR_STATEMENT: {
            VarStatement* stmt = (VarStatement*)node;
            free_ast(stmt->value);
            free_korelin_token(&stmt->name); // 释放 name Token
            free(stmt);
            break;
        }
        case NODE_RETURN_STATEMENT: {
            ReturnStatement* stmt = (ReturnStatement*)node;
            free_ast(stmt->return_value);
            free(stmt);
            break;
        }
        case NODE_EXPRESSION_STATEMENT: {
            ExpressionStatement* stmt = (ExpressionStatement*)node;
            free_ast(stmt->expression);
            free(stmt);
            break;
        }
        case NODE_BLOCK_STATEMENT: {
            BlockStatement* block = (BlockStatement*)node;
            for (size_t i = 0; i < block->statement_count; i++) {
                free_ast(block->statements[i]);
            }
            free(block->statements);
            free(block);
            break;
        }
        case NODE_IF_STATEMENT: {
            IfStatement* stmt = (IfStatement*)node;
            free_ast(stmt->condition);
            free_ast(stmt->consequence);
            free_ast(stmt->alternative);
            free(stmt);
            break;
        }
        case NODE_PROGRAM: {
            Program* program = (Program*)node;
            for (size_t i = 0; i < program->statement_count; i++) {
                free_ast(program->statements[i]);
            }
            free(program->statements);
            free(program);
            break;
        }
        // ... 为其他节点类型添加 case
        default:
            free(node);
            break;
    }
}