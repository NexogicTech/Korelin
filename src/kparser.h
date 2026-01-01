//
// Created by Helix on 2025/12/28.
//

#ifndef KORELIN_KPARSER_H
#define KORELIN_KPARSER_H

#include "ast.h"
#include "klexer.h"

/**
 * @brief 解析输入的源代码并返回一个 AST 的根节点 (Program)。
 * @param input 源代码字符串。
 * @return 指向 Program 节点的指针。调用者需要负责调用 free_ast 释放内存。
 */
Program* parse_program(const char* input);

 // void init_parser(KorelinParser * parser, const char* input);
#endif //KORELIN_KPARSER_H