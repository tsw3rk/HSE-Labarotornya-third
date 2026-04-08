#ifndef DERIVATES_NODE_H
#define DERIVATES_NODE_H

#pragma once
#include <string>
#include <memory>
#include <map>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>

using namespace std;

// Базовый класс узла дерева выражений
struct Node {
    virtual ~Node() = default;

    // Вычислить значение выражения при заданных переменных
    virtual double evaluate(const map<string, double>& vars) const = 0;

    // Взять производную по переменной var_name
    virtual shared_ptr<Node> derivative(const string& var_name) const = 0;

    // Преобразовать узел в строку (для вывода формулы)
    virtual string toString() const = 0;

    // Упростить выражение (убрать лишние скобки, 0+х, 1*х и т.д.)
    virtual shared_ptr<Node> simplify() const = 0;
};

// Узел: Число (константа)
struct NumberNode : public Node {
    double value;
    NumberNode(double val) : value(val) {}

    double evaluate(const map<string, double>& vars) const override;
    shared_ptr<Node> derivative(const string& var_name) const override;
    string toString() const override;
    shared_ptr<Node> simplify() const override;
};

// Узел: Переменная (x, y...)
struct VariableNode : public Node {
    string name;
    VariableNode(const string& n) : name(n) {}

    double evaluate(const map<string, double>& vars) const override;
    shared_ptr<Node> derivative(const string& var_name) const override;
    string toString() const override;
    shared_ptr<Node> simplify() const override;
};

// Узел: Бинарная операция (+, -, *, /, ^)
struct BinaryOpNode : public Node {
    char op;
    shared_ptr<Node> left;
    shared_ptr<Node> right;

    BinaryOpNode(char operation, shared_ptr<Node> l, shared_ptr<Node> r)
        : op(operation), left(l), right(r) {}

    double evaluate(const map<string, double>& vars) const override;
    shared_ptr<Node> derivative(const string& var_name) const override;
    string toString() const override;
    shared_ptr<Node> simplify() const override;
};

// Узел: Функция (sin, cos, sqrt...)
struct FunctionNode : public Node {
    string func_name;
    shared_ptr<Node> arg;

    FunctionNode(const string& name, shared_ptr<Node> a) : func_name(name), arg(a) {}

    double evaluate(const map<string, double>& vars) const override;
    shared_ptr<Node> derivative(const string& var_name) const override;
    string toString() const override;
    shared_ptr<Node> simplify() const override;
};

#endif //DERIVATES_NODE_H