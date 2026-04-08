#include "parser.h"

Parser::Parser(const string& expression) : expr(expression), pos(0) {}

void Parser::skipWhitespace() {
    while (pos < expr.size() && isspace(expr[pos])) pos++;
}

bool Parser::match(char c) {
    skipWhitespace();
    if (pos < expr.size() && expr[pos] == c) {
        pos++;
        return true;
    }
    return false;
}

char Parser::peek() {
    skipWhitespace();
    if (pos < expr.size()) return expr[pos];
    return '\0';
}

shared_ptr<Node> Parser::parseNumber() {
    size_t start = pos;
    // Проверка на начало с точки (например .2)
    if (pos < expr.size() && expr[pos] == '.') {
        throw runtime_error("ERROR Invalid number format");
    }

    while (pos < expr.size() && isdigit(expr[pos])) pos++;

    // Проверка на лидирующие нули (например 05)
    if (expr[start] == '0' && pos > start + 1 && isdigit(expr[start + 1])) {
        throw runtime_error("ERROR Invalid number format");
    }

    // Дробная часть
    if (pos < expr.size() && expr[pos] == '.') {
        pos++;
        // После точки обязательно должна быть цифра (например 2. -> ошибка)
        if (pos >= expr.size() || !isdigit(expr[pos])) {
            throw runtime_error("ERROR Invalid number format");
        }
        while (pos < expr.size() && isdigit(expr[pos])) pos++;
    }

    // Экспонента
    if (pos < expr.size() && (expr[pos] == 'e' || expr[pos] == 'E')) {
        pos++;
        if (pos < expr.size() && (expr[pos] == '+' || expr[pos] == '-')) pos++;
        if (pos >= expr.size() || !isdigit(expr[pos])) {
            throw runtime_error("ERROR Invalid number format");
        }
        while (pos < expr.size() && isdigit(expr[pos])) pos++;
    }

    if (pos == start) throw runtime_error("ERROR Expected number");
    string numStr = expr.substr(start, pos - start);
    try {
        return make_shared<NumberNode>(stod(numStr));
    } catch (...) {
        throw runtime_error("ERROR Invalid number format");
    }
}

string Parser::parseName() {
    size_t start = pos;
    while (pos < expr.size() && (isalnum(expr[pos]) || expr[pos] == '_')) pos++;
    if (pos == start) throw runtime_error("ERROR Expected name");
    return expr.substr(start, pos - start);
}

shared_ptr<Node> Parser::parseExpression() {
    shared_ptr<Node> node = parseTerm();
    while (true) {
        if (match('+')) node = make_shared<BinaryOpNode>('+', node, parseTerm());
        else if (match('-')) node = make_shared<BinaryOpNode>('-', node, parseTerm());
        else break;
    }
    return node;
}

shared_ptr<Node> Parser::parseTerm() {
    shared_ptr<Node> node = parseUnary(); // Вызываем parseUnary вместо parseFactor
    while (true) {
        if (match('*')) node = make_shared<BinaryOpNode>('*', node, parseUnary());
        else if (match('/')) node = make_shared<BinaryOpNode>('/', node, parseUnary());
        else break;
    }
    return node;
}

// Новый метод для унарных операторов
shared_ptr<Node> Parser::parseUnary() {
    skipWhitespace();
    if (pos < expr.size() && expr[pos] == '+') {
        pos++;
        return parseUnary();
    }
    if (pos < expr.size() && expr[pos] == '-') {
        pos++;
        auto operand = parseUnary();
        return make_shared<BinaryOpNode>('*', make_shared<NumberNode>(-1), operand);
    }
    return parseFactor();
}

shared_ptr<Node> Parser::parseFactor() {
    shared_ptr<Node> node = parsePrimary(); // Левая часть: только Primary (без унарных)
    if (match('^')) {
        shared_ptr<Node> right = parseUnary(); // Правая часть: Unary (разрешаем унарные)
        node = make_shared<BinaryOpNode>('^', node, right);
    }
    return node;
}

shared_ptr<Node> Parser::parsePrimary() {
    skipWhitespace();
    if (pos >= expr.size()) throw runtime_error("ERROR Unexpected end of expression");

    // Скобки
    if (expr[pos] == '(') {
        pos++;
        shared_ptr<Node> node = parseExpression();
        if (!match(')')) throw runtime_error("ERROR Missing closing parenthesis");
        return node;
    }

    // Число
    if (isdigit(expr[pos])) return parseNumber();

    // Имя (функция или переменная)
    if (isalpha(expr[pos]) || expr[pos] == '_') {
        string name = parseName();
        transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return tolower(c); });
        vector<string> funcs = {"sin", "cos", "tan", "tg", "ctg", "cot", "asin", "acos", "atan", "exp", "log", "sqrt"};
        if (find(funcs.begin(), funcs.end(), name) != funcs.end()) {
            if (!match('(')) throw runtime_error("ERROR Expected '(' after function " + name);
            shared_ptr<Node> arg = parseExpression();
            if (!match(')')) throw runtime_error("ERROR Missing closing parenthesis for function " + name);
            return make_shared<FunctionNode>(name, arg);
        } else {
            return make_shared<VariableNode>(name);
        }
    }
    throw runtime_error("ERROR Unexpected character: " + string(1, expr[pos]));
}

shared_ptr<Node> Parser::parse() {
    shared_ptr<Node> root = parseExpression();
    skipWhitespace();
    if (pos != expr.size()) throw runtime_error("ERROR Unexpected characters at end of expression");
    return root;
}