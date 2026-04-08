#include <catch2/catch_all.hpp>
#include <iostream>
#include <string>
#include <map>
#include <cmath>
#include <limits>
#include <vector>

#include "node.h"
#include "parser.h"

using namespace std;

double run_evaluate(const string& expr, const map<string, double>& vars) {
    Parser parser(expr);
    shared_ptr<Node> root = parser.parse();
    return root->evaluate(vars);
}

double run_evaluate_derivative(const string& expr, const string& var_name, const map<string, double>& vars) {
    Parser parser(expr);
    shared_ptr<Node> root = parser.parse();
    shared_ptr<Node> deriv = root->derivative(var_name);
    return deriv->evaluate(vars);
}

string run_symbolic_derivative(const string& expr, const string& var_name) {
    Parser parser(expr);
    shared_ptr<Node> root = parser.parse();
    shared_ptr<Node> deriv = root->derivative(var_name);
    deriv = deriv->simplify();
    return deriv->toString();
}

double numerical_derivative(const string& expr, const string& var_name, double val, const map<string, double>& extra_vars) {
    double h = 1e-6;
    map<string, double> vars_plus = extra_vars;
    map<string, double> vars_minus = extra_vars;
    vars_plus[var_name] = val + h;
    vars_minus[var_name] = val - h;
    double f_plus = run_evaluate(expr, vars_plus);
    double f_minus = run_evaluate(expr, vars_minus);
    return (f_plus - f_minus) / (2 * h);
}

void check_double(double actual, double expected) {
    if (std::isinf(actual) && std::isinf(expected)) {
        REQUIRE((actual > 0) == (expected > 0));
    } else if (std::isnan(actual) && std::isnan(expected)) {
        // OK
    } else {
        REQUIRE(actual == Catch::Approx(expected).epsilon(1e-5));
    }
}

struct EvalTestCase {
    string expr;
    map<string, double> vars;
    double expected;
};

struct DerivEvalTestCase {
    string expr;
    string var;
    double val;
    double expected_deriv;
};

struct SymbolicDerivTestCase {
    string expr;
    string var;
    double test_val;
};

TEST_CASE("Simple Evaluation Examples", "[eval]") {
    vector<EvalTestCase> tests = {
        {"1 + 1", {}, 2.0}, {"1+1", {}, 2.0}, {"1 +1", {}, 2.0}, {"1+ 1", {}, 2.0},
        {"1       +       1", {}, 2.0}, {"0", {}, 0.0}, {"0+0", {}, 0.0}, {"0/1", {}, 0.0},
        {"5/0", {}, std::numeric_limits<double>::infinity()}, {"2 + 4 * 5", {}, 22.0},
        {"2+ 4 * 5", {}, 22.0}, {"2 + 4* 5", {}, 22.0}, {"2 +4* 5", {}, 22.0},
        {"6/3", {}, 2.0}, {"5/2", {}, 2.5}, {"2/5", {}, 0.4}, {"2 * 2 + 5", {}, 9.0},
        {"2^2 + 5", {}, 9.0}, {"2^2*5", {}, 20.0}, {"2 + 5*2^4", {}, 82.0},
        {"(2 + 5)*2^4", {}, 112.0}, {"2 + (5 * 2) ^ 4", {}, 10002.0},
        {"2 + 5 * (2 ^ 4)", {}, 82.0}, {"(2 + 5 * 2) ^ 4", {}, 20736.0},
        {"((2 + 5) * 2) ^ 4", {}, 38416.0}, {"6/(3)", {}, 2.0}, {"(6/3)", {}, 2.0},
        {"-3 + 3", {}, 0.0}, {"-2 + 5", {}, 3.0}, {"-2 * 4", {}, -8.0},
        {"-2^2", {}, -4.0}, {"2 + -2^2", {}, -2.0}, {"2 * -2^2", {}, -8.0},
        {"2^-2", {}, 0.25}, {"(-2)^2", {}, 4.0}, {"2^-1", {}, 0.5}, {"4^-2", {}, 0.0625},
        {"(-2)^3", {}, -8.0}, {"(-2)^5", {}, -32.0}, {"2^3/2", {}, 4.0},
        {"2^2^2^2^2", {}, std::numeric_limits<double>::infinity()},
        {"1+1+1+1+1+1+1+1+1+1", {}, 10.0}, {"1-2+3-4+5-6+7-8+9", {}, 5.0},
        {"2^3^2", {}, 512.0}, {"3^2^2", {}, 81.0}, {"2^2^2^2", {}, 65536.0},
        {"+2", {}, 2.0}, {"++2", {}, 2.0}, {"+-2", {}, -2.0}, {"-+2", {}, -2.0},
        {"--2", {}, 2.0}, {"---2", {}, -2.0}, {"----2", {}, 2.0}, {"2 + --2", {}, 4.0},
        {"2 + ---2", {}, 0.0}, {"(-3)", {}, -3.0}, {"(+3)", {}, 3.0},
        {"-(3)", {}, -3.0}, {"+(3)", {}, 3.0}, {"2*-3", {}, -6.0}, {"2*+3", {}, 6.0},
        {"2--3", {}, 5.0}, {"2+-3", {}, -1.0}, {"((((2))))", {}, 2.0},
        {"((((((3+4))))))", {}, 7.0}, {"2 + 3 * 4 + 5 * 6 + 7 * 8", {}, 100.0},
        {"(2 + 3) * (4 + 5) * (6 + 7)", {}, 585.0}, {"2 + 3 * 4 ^ 2 / 5 - 6", {}, 5.6},
        {"0.5 + 0.25", {}, 0.75}, {"2.5 * 4", {}, 10.0}, {"1.2^2", {}, 1.44},
        {"0*5", {}, 0.0}, {"5*0", {}, 0.0}, {"0^2", {}, 0.0}, {"2^0", {}, 1.0},
        {"5^1", {}, 5.0}, {"5*1", {}, 5.0}, {"5+0", {}, 5.0}, {"5-0", {}, 5.0},
        {"-0.5", {}, -0.5}, {"(-0.5)", {}, -0.5}, {"2*-0.5", {}, -1.0},
        {"0.5^2", {}, 0.25}, {"2^0.5", {}, 1.41421356}
    };
    for (const auto& t : tests) {
        INFO("Testing expression: " << t.expr);
        double result = run_evaluate(t.expr, t.vars);
        check_double(result, t.expected);
    }
}

TEST_CASE("Variable Evaluation Examples", "[eval_vars]") {
    vector<EvalTestCase> tests = {
        {"x + y + z", {{"x", 1}, {"y", 2}, {"z", 3}}, 6.0},
        {"x^y + y^x", {{"x", 2}, {"y", 3}}, 17.0},
        {"x^2 + x", {{"x", -2}}, 2.0},
        {"x + y*z + x^y + z^x", {{"x", 2}, {"y", 3}, {"z", 4}}, 38.0},
        {"x + y*z + x^y + z^x", {{"x", 2}, {"y", -3.2}, {"z", 4}}, 5.30882},
        {"x + y*z + x^y + z^x", {{"x", 0.1}, {"y", -3.2}, {"z", 4.5}}, 1571.76},
        {"sin(x)", {{"x", 0}}, 0.0},
        {"sin(cos(x))", {{"x", 1}}, 0.514395},
        {"sin(x) + y^2", {{"x", 1}, {"y", 2}}, 4.84147},
        {"sin(x)^2", {{"x", 2}}, 0.826822},
        {"sin(x^2)", {{"x", 2}}, -0.756802},
        {"sqrt(z) + log(y) + cos(x)", {{"x", 1}, {"y", 2}, {"z", 4}}, 3.23345},
        {"SiN(x)", {{"x", 1}}, 0.841471},
        {"sQrT(x^2)", {{"x", 1}}, 1.0},
        {"log(x)", {{"x", 0}}, -std::numeric_limits<double>::infinity()},
        {"sin(x) + cos(y^2) * sqrt(z + x^y)", {{"x", 1}, {"y", 2}, {"z", 3}}, -0.465816}
    };
    for (const auto& t : tests) {
        INFO("Testing expression: " << t.expr);
        double result = run_evaluate(t.expr, t.vars);
        check_double(result, t.expected);
    }
}

TEST_CASE("Derivative Evaluation Examples", "[eval_deriv]") {
    vector<DerivEvalTestCase> tests = {
        {"5", "x", 10, 0.0}, {"x^2", "x", 2, 4.0}, {"x^3", "x", 3, 27.0},
        {"x*x", "x", 2, 4.0}, {"x*(x+1)", "x", 3, 7.0}, {"1/x", "x", 2, -0.25},
        {"sin(x)", "x", 0, 1.0}, {"cos(x)", "x", 0, 0.0}, {"log(x)", "x", 1, 1.0},
        {"sqrt(x)", "x", 4, 0.25}, {"sin(x^2)", "x", 2, -2.61457},
        {"sin(2*x)", "x", 1, -0.832294}, {"x*y", "x", 2, 1.0},
        {"sin(x*y)", "x", 1, 0.540302}, {"y^2", "x", 5, 0.0},
        {"x^3", "x", -2, 12.0}, {"x^2", "x", 0.5, 1.0}
    };
    for (const auto& t : tests) {
        INFO("Testing derivative of: " << t.expr << " wrt " << t.var << " at " << t.val);
        map<string, double> vars = {{t.var, t.val}};
        if (t.expr.find("y") != string::npos && vars.count("y") == 0) vars["y"] = 1.0;
        if (t.expr.find("z") != string::npos && vars.count("z") == 0) vars["z"] = 1.0;
        double result = run_evaluate_derivative(t.expr, t.var, vars);
        check_double(result, t.expected_deriv);
    }
}

TEST_CASE("Symbolic Derivative Verification", "[sym_deriv]") {
    vector<SymbolicDerivTestCase> tests = {
        {"5", "x", 1.0}, {"x", "x", 1.0}, {"y", "x", 1.0}, {"x + x", "x", 1.0},
        {"x^2", "x", 2.0}, {"x^3", "x", 2.0}, {"3*x^2", "x", 2.0},
        {"x*x", "x", 3.0}, {"x*(x+1)", "x", 2.0}, {"1/x", "x", 2.0},
        {"x/(x+1)", "x", 1.0}, {"sin(x)", "x", 1.0}, {"cos(x)", "x", 1.0},
        {"log(x)", "x", 2.0}, {"sqrt(x)", "x", 4.0}, {"sin(2*x)", "x", 1.0},
        {"sin(x^2)", "x", 1.0}, {"log(x^2)", "x", 2.0},
        {"x^2 + sin(x*y) + y^2", "x", 1.0}, {"((((x^2))))", "x", 3.0},
        {"sin(x*x)", "x", 1.0}, {"sin(x*x)*cos(x*x)", "x", 1.0},
        {"sin(cos(log(x*x)))", "x", 1.0}, {"sqrt(sqrt(x))", "x", 4.0},
        {"sqrt(x^2)", "x", 2.0}, {"sqrt(sin(x))", "x", 1.0}, {"sqrt(2*x)", "x", 2.0}
    };
    for (const auto& t : tests) {
        INFO("Verifying symbolic derivative of: " << t.expr);
        string sym_deriv_str = run_symbolic_derivative(t.expr, t.var);
        Parser res_parser(sym_deriv_str);
        shared_ptr<Node> deriv_tree = res_parser.parse();
        map<string, double> vars = {{t.var, t.test_val}};
        if (t.expr.find("y") != string::npos) vars["y"] = 1.0;
        if (t.expr.find("z") != string::npos) vars["z"] = 1.0;
        double student_val = deriv_tree->evaluate(vars);
        double expected_val = numerical_derivative(t.expr, t.var, t.test_val, vars);
        check_double(student_val, expected_val);
    }
}

TEST_CASE("Auto Generated Simple Tests", "[auto_simple]") {
    for (int i = 0; i < 90; ++i) {
        string expr = to_string(i) + " + " + to_string(i+1);
        double expected = 2*i + 1;
        CHECK(run_evaluate(expr, {}) == Catch::Approx(expected));
    }
    for (int i = 0; i < 90; ++i) {
        string expr = to_string(i) + " * 2";
        double expected = i * 2.0;
        CHECK(run_evaluate(expr, {}) == Catch::Approx(expected));
    }
}

TEST_CASE("Auto Generated Variable Tests", "[auto_vars]") {
    for (int i = 0; i < 90; ++i) {
        string expr = "x + " + to_string(i);
        map<string, double> vars = {{"x", 5.0}};
        double expected = 5.0 + i;
        CHECK(run_evaluate(expr, vars) == Catch::Approx(expected));
    }
    for (int i = 1; i <= 90; ++i) {
        string expr = "sin(x) * " + to_string(i);
        map<string, double> vars = {{"x", 0.0}};
        double expected = 0.0;
        CHECK(run_evaluate(expr, vars) == Catch::Approx(expected));
    }
}

TEST_CASE("Auto Generated Derivative Tests", "[auto_deriv]") {
    for (int n = 1; n <= 90; ++n) {
        string expr = "x^" + to_string(n);
        string var = "x";
        double val = 2.0;
        double expected = n * pow(2.0, n - 1);
        map<string, double> vars = {{var, val}};
        double result = run_evaluate_derivative(expr, var, vars);
        CHECK(result == Catch::Approx(expected).margin(1e-3 * abs(expected)));
    }
}