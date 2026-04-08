#include "node.h"

// --- NumberNode Implementation ---

double NumberNode::evaluate(const map<string, double>& vars) const {
    return value;
}

shared_ptr<Node> NumberNode::derivative(const string& var_name) const {
    return make_shared<NumberNode>(0.0);
}

string NumberNode::toString() const {
    // Если число целое, выводим без .0
    if (value == static_cast<int>(value) && abs(value) < 1e9) {
        return to_string(static_cast<int>(value));
    }
    // Иначе стандартный вывод, но можно настроить точность
    ostringstream oss;
    oss << value;
    return oss.str();
}

shared_ptr<Node> NumberNode::simplify() const {
    return make_shared<NumberNode>(value);
}

// --- VariableNode Implementation ---

double VariableNode::evaluate(const map<string, double>& vars) const {
    auto it = vars.find(name);
    if (it == vars.end()) {
        throw runtime_error("ERROR Unknown variable: " + name);
    }
    return it->second;
}

shared_ptr<Node> VariableNode::derivative(const string& var_name) const {
    if (name == var_name) {
        return make_shared<NumberNode>(1.0);
    }
    return make_shared<NumberNode>(0.0);
}

string VariableNode::toString() const {
    return name;
}

shared_ptr<Node> VariableNode::simplify() const {
    return make_shared<VariableNode>(name);
}

// --- BinaryOpNode Implementation ---

double BinaryOpNode::evaluate(const map<string, double>& vars) const {
    double l_val = left->evaluate(vars);
    double r_val = right->evaluate(vars);

    switch (op) {
        case '+': return l_val + r_val;
        case '-': return l_val - r_val;
        case '*': return l_val * r_val;
        case '/':
            return l_val / r_val;
        case '^':
            if (l_val == 0 && r_val <= 0) throw runtime_error("ERROR Domain error: 0^non-positive");
            return pow(l_val, r_val);
        default: throw runtime_error("ERROR Unknown operator");
    }
}

shared_ptr<Node> BinaryOpNode::derivative(const string& var_name) const {
    shared_ptr<Node> d_left = left->derivative(var_name);
    shared_ptr<Node> d_right = right->derivative(var_name);

    if (op == '+') {
        return make_shared<BinaryOpNode>('+', d_left, d_right);
    } else if (op == '-') {
        return make_shared<BinaryOpNode>('-', d_left, d_right);
    } else if (op == '*') {
        // (uv)' = u'v + uv'
        auto term1 = make_shared<BinaryOpNode>('*', d_left, right);
        auto term2 = make_shared<BinaryOpNode>('*', left, d_right);
        return make_shared<BinaryOpNode>('+', term1, term2);
    } else if (op == '/') {
        // (u/v)' = (u'v - uv') / v^2
        auto num_left = make_shared<BinaryOpNode>('*', d_left, right);
        auto num_right = make_shared<BinaryOpNode>('*', left, d_right);
        auto numerator = make_shared<BinaryOpNode>('-', num_left, num_right);
        auto denominator = make_shared<BinaryOpNode>('^', right, make_shared<NumberNode>(2));
        return make_shared<BinaryOpNode>('/', numerator, denominator);
    } else if (op == '^') {
        // Проверяем, является ли правая часть (степень) константой
        if (auto rightNum = dynamic_cast<NumberNode*>(right.get())) {
            // Формула для константной степени: (u^n)' = n * u^(n-1) * u'
            double n = rightNum->value;
            auto new_exp = make_shared<BinaryOpNode>('^', left, make_shared<NumberNode>(n - 1.0));
            auto coeff = make_shared<NumberNode>(n);
            auto term = make_shared<BinaryOpNode>('*', coeff, new_exp);
            return make_shared<BinaryOpNode>('*', term, d_left);
        } else {
            // Общая формула: (u^v)' = u^v * (v' * ln(u) + v * u' / u)
            auto ln_u = make_shared<FunctionNode>("log", left);
            auto part1 = make_shared<BinaryOpNode>('*', d_right, ln_u);
            auto u_div_u = make_shared<BinaryOpNode>('/', d_left, left);
            auto part2 = make_shared<BinaryOpNode>('*', right, u_div_u);
            auto inner_sum = make_shared<BinaryOpNode>('+', part1, part2);
            auto base_pow = make_shared<BinaryOpNode>('^', left, right);
            return make_shared<BinaryOpNode>('*', base_pow, inner_sum);
        }
    }
    throw runtime_error("ERROR Derivative not implemented for operator");
}

string BinaryOpNode::toString() const {
    return "(" + left->toString() + " " + op + " " + right->toString() + ")";
}

shared_ptr<Node> BinaryOpNode::simplify() const {
    auto new_left = left->simplify();
    auto new_right = right->simplify();

    // Попытка упростить константы
    auto num_l = dynamic_cast<NumberNode*>(new_left.get());
    auto num_r = dynamic_cast<NumberNode*>(new_right.get());

    if (num_l && num_r) {
        // Если оба операнда числа, считаем результат сразу
        try {
            double res = 0;
            switch(op) {
                case '+': res = num_l->value + num_r->value; break;
                case '-': res = num_l->value - num_r->value; break;
                case '*': res = num_l->value * num_r->value; break;
                case '/':
                    if(num_r->value == 0) throw runtime_error("Div0");
                    res = num_l->value / num_r->value; break;
                case '^': res = pow(num_l->value, num_r->value); break;
            }
            return make_shared<NumberNode>(res);
        } catch (...) {
            // Если ошибка (деление на 0), оставляем как есть, ошибка всплывет при evaluate
        }
    }

    // Правила идентичности
    if (op == '+') {
        if (num_l && num_l->value == 0) return new_right;
        if (num_r && num_r->value == 0) return new_left;
    }
    if (op == '-') {
        if (num_r && num_r->value == 0) return new_left;
    }
    if (op == '*') {
        if (num_l && num_l->value == 0) return make_shared<NumberNode>(0);
        if (num_r && num_r->value == 0) return make_shared<NumberNode>(0);
        if (num_l && num_l->value == 1) return new_right;
        if (num_r && num_r->value == 1) return new_left;
    }
    if (op == '/') {
        if (num_l && num_l->value == 0) return make_shared<NumberNode>(0);
        if (num_r && num_r->value == 1) return new_left;
    }
    if (op == '^') {
        if (num_r && num_r->value == 0) return make_shared<NumberNode>(1);
        if (num_r && num_r->value == 1) return new_left;
        if (num_l && num_l->value == 0) return make_shared<NumberNode>(0); // Упрощенно
    }

    return make_shared<BinaryOpNode>(op, new_left, new_right);
}

// --- FunctionNode Implementation ---

double FunctionNode::evaluate(const map<string, double>& vars) const {
    double val = arg->evaluate(vars);
    if (func_name == "sin") return sin(val);
    if (func_name == "cos") return cos(val);
    if (func_name == "tan" || func_name == "tg") return tan(val);
	if (func_name == "ctg" || func_name == "cot") {
        double sin_val = sin(val);
        if (abs(sin_val) < 1e-12) throw runtime_error("ERROR Domain error: ctg (sin(x) is zero)");
        return cos(val) / sin_val;
	}
    if (func_name == "asin") {
        if (val < -1 || val > 1) throw runtime_error("ERROR Domain error: asin");
        return asin(val);
    }
    if (func_name == "acos") {
        if (val < -1 || val > 1) throw runtime_error("ERROR Domain error: acos");
        return acos(val);
    }
    if (func_name == "atan") return atan(val);
    if (func_name == "exp") return exp(val);
    if (func_name == "log") {
        if (val < 0) throw runtime_error("ERROR Domain error: log");
        return log(val);
    }
    if (func_name == "sqrt") {
        if (val < 0) throw runtime_error("ERROR Domain error: sqrt");
        return sqrt(val);
    }
    throw runtime_error("ERROR Unknown function: " + func_name);
}

shared_ptr<Node> FunctionNode::derivative(const string& var_name) const {
    shared_ptr<Node> d_arg = arg->derivative(var_name);
    shared_ptr<Node> inner_deriv = nullptr;

    if (func_name == "sin") {
        inner_deriv = make_shared<FunctionNode>("cos", arg);
    } else if (func_name == "cos") {
        inner_deriv = make_shared<BinaryOpNode>('*', make_shared<NumberNode>(-1), make_shared<FunctionNode>("sin", arg));
    } else if (func_name == "tan"|| func_name == "tg") {
        // (tan u)' = 1/cos^2(u) * u'
        auto cos_u = make_shared<FunctionNode>("cos", arg);
        auto denom = make_shared<BinaryOpNode>('^', cos_u, make_shared<NumberNode>(2));
        inner_deriv = make_shared<BinaryOpNode>('/', make_shared<NumberNode>(1), denom);
	} else if (func_name == "ctg" || func_name == "cot") {
        // (ctg(u))' = -1 / sin^2(u) * u'
        auto sin_u = make_shared<FunctionNode>("sin", arg);
        auto sin_sq = make_shared<BinaryOpNode>('^', sin_u, make_shared<NumberNode>(2));
        auto neg_one = make_shared<NumberNode>(-1.0);
        auto frac = make_shared<BinaryOpNode>('/', neg_one, sin_sq);
        inner_deriv = make_shared<BinaryOpNode>('*', frac, d_arg);
    } else if (func_name == "asin") {
        // (asin u)' = 1/sqrt(1-u^2) * u'
        auto u_sq = make_shared<BinaryOpNode>('^', arg, make_shared<NumberNode>(2));
        auto one_minus_u_sq = make_shared<BinaryOpNode>('-', make_shared<NumberNode>(1), u_sq);
        auto denom = make_shared<FunctionNode>("sqrt", one_minus_u_sq);
        inner_deriv = make_shared<BinaryOpNode>('/', make_shared<NumberNode>(1), denom);
    } else if (func_name == "acos") {
        // (acos u)' = -1/sqrt(1-u^2) * u'
        auto u_sq = make_shared<BinaryOpNode>('^', arg, make_shared<NumberNode>(2));
        auto one_minus_u_sq = make_shared<BinaryOpNode>('-', make_shared<NumberNode>(1), u_sq);
        auto denom = make_shared<FunctionNode>("sqrt", one_minus_u_sq);
        inner_deriv = make_shared<BinaryOpNode>('*', make_shared<NumberNode>(-1), make_shared<BinaryOpNode>('/', make_shared<NumberNode>(1), denom));
    } else if (func_name == "atan") {
        // (atan u)' = 1/(1+u^2) * u'
        auto u_sq = make_shared<BinaryOpNode>('^', arg, make_shared<NumberNode>(2));
        auto denom = make_shared<BinaryOpNode>('+', make_shared<NumberNode>(1), u_sq);
        inner_deriv = make_shared<BinaryOpNode>('/', make_shared<NumberNode>(1), denom);
    } else if (func_name == "exp") {
        inner_deriv = make_shared<FunctionNode>("exp", arg);
    } else if (func_name == "log") {
        inner_deriv = make_shared<BinaryOpNode>('/', make_shared<NumberNode>(1), arg);
    } else if (func_name == "sqrt") {
        // (sqrt u)' = 1/(2*sqrt(u)) * u'
        auto denom = make_shared<BinaryOpNode>('*', make_shared<NumberNode>(2), make_shared<FunctionNode>("sqrt", arg));
        inner_deriv = make_shared<BinaryOpNode>('/', make_shared<NumberNode>(1), denom);
    } else {
        throw runtime_error("ERROR Derivative not implemented for: " + func_name);
    }

    return make_shared<BinaryOpNode>('*', inner_deriv, d_arg);
}

string FunctionNode::toString() const {
    return func_name + "(" + arg->toString() + ")";
}

shared_ptr<Node> FunctionNode::simplify() const {
    auto new_arg = arg->simplify();
    return make_shared<FunctionNode>(func_name, new_arg);
}