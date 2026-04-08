#ifndef DERIVATES_PARSER_H
#define DERIVATES_PARSER_H

#pragma once
#include "node.h"

class Parser {
private:
    string expr;
    size_t pos;

    void skipWhitespace();
    bool match(char c);
    char peek();

    shared_ptr<Node> parseNumber();
    string parseName();

    // Грамматика рекурсивного спуска
    shared_ptr<Node> parseExpression(); // +, -
    shared_ptr<Node> parseTerm();       // *, /
    shared_ptr<Node> parseFactor();     // ^
    shared_ptr<Node> parseUnary();      // Унарные +, -
    shared_ptr<Node> parsePrimary();    // Числа, переменные, функции, скобки

public:
    Parser(const string& expression);
    shared_ptr<Node> parse();
};

#endif //DERIVATES_PARSER_H