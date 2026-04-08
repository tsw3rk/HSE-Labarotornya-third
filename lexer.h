#ifndef LAB3_LEXER_H
#define LAB3_LEXER_H

#pragma once
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

enum class TokenType {
    NUMBER,
    IDENT,
    PLUS,
    MINUS,
    MUL,
    DIV,
    POW,
    LPAREN,
    RPAREN,
    ERROR,
    END
};

struct Token {
    TokenType type;
    string value;

    Token(TokenType t = TokenType::END, const string& v = "") : type(t), value(v) {}
};

class Lexer {
private:
    string input;
    size_t pos;

    void skipWhitespace();
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNum(char c) const;
    Token readNumber();
    Token readIdentifier();

public:
    explicit Lexer(const string& str);
    vector<Token> tokenize();
    static string tokenTypeToString(TokenType type);
};

#endif //LAB3_LEXER_H