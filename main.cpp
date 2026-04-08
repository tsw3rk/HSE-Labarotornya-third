#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <cctype>
#include "parser.h"

using namespace std;

int main() {
    // Отключаем синхронизацию для скорости
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string command;
    while (cin >> command) {
        try {
            if (command != "evaluate" && command != "derivative" && command != "evaluate_derivative") {
                continue;
            }

            int n;
            cin >> n;
            if (n < 0) throw runtime_error("ERROR Invalid number of variables");

            vector<string> varNames(n);
            for (int i = 0; i < n; ++i) {
                cin >> varNames[i];
                // Приводим имена переменных к нижнему регистру
                transform(varNames[i].begin(), varNames[i].end(), varNames[i].begin(), ::tolower);
            }

            map<string, double> vars;
            for (int i = 0; i < n; ++i) {
                double val;
                cin >> val;
                vars[varNames[i]] = val;
            }

            // Очищаем буфер ввода от оставшегося символа новой строки после последнего числа
            string dummy;
            getline(cin, dummy);

            // Теперь читаем само выражение
            string exprLine;
            getline(cin, exprLine);

            // Убираем пробелы по краям, если они есть
            size_t start = exprLine.find_first_not_of(" \t\r");
            size_t end = exprLine.find_last_not_of(" \t\r");

            if (start == string::npos) {
                 // Если строка пустая или состоит только из пробелов
                 throw runtime_error("ERROR Empty expression");
            }

            exprLine = exprLine.substr(start, end - start + 1);

            if (exprLine.empty()) {
                throw runtime_error("ERROR Empty expression");
            }

            // Парсим и упрощаем
            Parser parser(exprLine);
            shared_ptr<Node> root = parser.parse();
            root = root->simplify();

            // Выполняем команду
            if (command == "evaluate") {
                double res = root->evaluate(vars);
                cout << res << endl;
            }
            else if (command == "derivative") {
                if (n == 0) throw runtime_error("ERROR No variables for derivative");
                string diffVar = varNames[0];
                shared_ptr<Node> derivTree = root->derivative(diffVar);
                derivTree = derivTree->simplify();
                cout << derivTree->toString() << endl;
            }
            else if (command == "evaluate_derivative") {
                if (n == 0) throw runtime_error("ERROR No variables for derivative");
                string diffVar = varNames[0];
                shared_ptr<Node> derivTree = root->derivative(diffVar);
                derivTree = derivTree->simplify();
                double res = derivTree->evaluate(vars);
                cout << res << endl;
            }
        } catch (const exception& e) {
            cout << e.what() << endl;
        }
    }

    return 0;
}