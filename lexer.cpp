#include "lexer.h"
#include <stdexcept>
#include <sstream>

Lexer::Lexer(const string& str) : input(str), pos(0) {}

void Lexer::skipWhitespace() {
    while (pos < input.size() && isspace(static_cast<unsigned char>(input[pos]))) {
        pos++;
    }
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNum(char c) const {
    return isAlpha(c) || isDigit(c);
}

Token Lexer::readNumber() {
    size_t start = pos;
    bool hasDot = false;

    // Проверяем ведущие нули
    if (pos < input.size() && input[pos] == '0') {
        pos++;
        // Если после 0 идет еще цифра (не точка и не экспонента) - это ошибка
        if (pos < input.size() && isDigit(input[pos])) {
            return Token(TokenType::ERROR, "");
        }
        // Если это просто 0
        if (pos >= input.size() || (!isDigit(input[pos]) && input[pos] != '.' &&
            input[pos] != 'e' && input[pos] != 'E')) {
            return Token(TokenType::NUMBER, "0");
        }
    }

    // Целая часть
    while (pos < input.size() && isDigit(input[pos])) {
        pos++;
    }

    // Дробная часть
    if (pos < input.size() && input[pos] == '.') {
        hasDot = true;
        pos++;
        while (pos < input.size() && isDigit(input[pos])) {
            pos++;
        }
    }

    // Экспонента
    if (pos < input.size() && (input[pos] == 'e' || input[pos] == 'E')) {
        pos++;
        if (pos < input.size() && (input[pos] == '+' || input[pos] == '-')) {
            pos++;
        }
        // После e/+/- должны быть цифры
        if (pos >= input.size() || !isDigit(input[pos])) {
            return Token(TokenType::ERROR, "");
        }
        while (pos < input.size() && isDigit(input[pos])) {
            pos++;
        }
    }

    string numStr = input.substr(start, pos - start);
    return Token(TokenType::NUMBER, numStr);
}

Token Lexer::readIdentifier() {
    size_t start = pos;

    while (pos < input.size() && isAlphaNum(input[pos])) {
        pos++;
    }

    string name = input.substr(start, pos - start);
    // Приводим к нижнему регистру
    transform(name.begin(), name.end(), name.begin(),
              [](unsigned char c) { return tolower(c); });

    return Token(TokenType::IDENT, name);
}

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;

    while (pos < input.size()) {
        skipWhitespace();
        if (pos >= input.size()) break;

        char c = input[pos];

        // Скобки
        if (c == '(') {
            tokens.push_back(Token(TokenType::LPAREN, "("));
            pos++;
            continue;
        }
        if (c == ')') {
            tokens.push_back(Token(TokenType::RPAREN, ")"));
            pos++;
            continue;
        }

        // Операторы
        if (c == '+') {
            tokens.push_back(Token(TokenType::PLUS, "+"));
            pos++;
            continue;
        }
        if (c == '-') {
            tokens.push_back(Token(TokenType::MINUS, "-"));
            pos++;
            continue;
        }
        if (c == '*') {
            tokens.push_back(Token(TokenType::MUL, "*"));
            pos++;
            continue;
        }
        if (c == '/') {
            tokens.push_back(Token(TokenType::DIV, "/"));
            pos++;
            continue;
        }
        if (c == '^') {
            tokens.push_back(Token(TokenType::POW, "^"));
            pos++;
            continue;
        }

        // Числа
        if (isDigit(c) || c == '.') {
            Token numToken = readNumber();
            if (numToken.type == TokenType::ERROR) {
                return {}; // Ошибка
            }
            // Проверяем, что после числа не идет буква
            if (pos < input.size() && isAlpha(input[pos])) {
                return {}; // Ошибка: 2s3
            }
            tokens.push_back(numToken);
            continue;
        }

        // Идентификаторы
        if (isAlpha(c)) {
            tokens.push_back(readIdentifier());
            continue;
        }

        // Неизвестный символ
        return {};
    }

    return tokens;
}

string Lexer::tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::IDENT: return "IDENT";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MUL: return "MUL";
        case TokenType::DIV: return "DIV";
        case TokenType::POW: return "POW";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::ERROR: return "ERROR";
        case TokenType::END: return "END";
        default: return "UNKNOWN";
    }
}