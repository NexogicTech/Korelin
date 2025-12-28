//
// Created by Helix on 2025/12/28.
//

#ifndef KORELIN_LEXER_H
#define KORELIN_LEXER_H

typedef enum {
    // Special Token
    KORELIN_EOF,
    KORELIN_ERROR,

    // 分隔符
    KORELIN_COMMA,     // ,
    KORELIN_SEMICOLON, // ;
    TOKEN_LPAREN, // (
    TOKEN_RPAREN, // )
    TOKEN_LBRACE, // {
    TOKEN_RBRACE, // }

} TokenType;

#endif //KORELIN_LEXER_H