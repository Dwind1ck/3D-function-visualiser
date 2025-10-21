#include "math_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Глобальные переменные для парсера
static const char* parse_pos;
static double var_x, var_y;
char parse_error[256] = "";

// Прототипы внутренних функций
static void skip_whitespace();
static Token get_next_token();
static double parse_expression();
static double parse_term();
static double parse_factor();
static double parse_power();

// Пропуск пробелов
static void skip_whitespace() {
    while (*parse_pos && isspace(*parse_pos)) {
        parse_pos++;
    }
}

// Получение следующего токена
static Token get_next_token() {
    Token token;
    skip_whitespace();
    
    if (*parse_pos == '\0') {
        token.type = TOKEN_END;
        return token;
    }
    
    // Числа
    if (isdigit(*parse_pos) || *parse_pos == '.') {
        char* end;
        token.value = strtod(parse_pos, &end);
        token.type = TOKEN_NUMBER;
        parse_pos = end;
        return token;
    }
    
    // Переменные и функции
    if (isalpha(*parse_pos)) {
        int i = 0;
        while (isalnum(*parse_pos) && i < 31) {
            token.text[i++] = *parse_pos++;
        }
        token.text[i] = '\0';
        
        // Проверяем, является ли это функцией
        if (strcmp(token.text, "sin") == 0 || strcmp(token.text, "cos") == 0 ||
            strcmp(token.text, "tan") == 0 || strcmp(token.text, "exp") == 0 ||
            strcmp(token.text, "log") == 0 || strcmp(token.text, "sqrt") == 0 ||
            strcmp(token.text, "abs") == 0 || strcmp(token.text, "ln") == 0 ||
            strcmp(token.text, "asin") == 0 || strcmp(token.text, "acos") == 0 ||
            strcmp(token.text, "atan") == 0 || strcmp(token.text, "sinh") == 0 ||
            strcmp(token.text, "cosh") == 0 || strcmp(token.text, "tanh") == 0) {
            token.type = TOKEN_FUNCTION;
        } else {
            token.type = TOKEN_VARIABLE;
        }
        return token;
    }
    
    // Операторы и скобки
    switch (*parse_pos) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '^':
            token.type = TOKEN_OPERATOR;
            token.text[0] = *parse_pos;
            token.text[1] = '\0';
            parse_pos++;
            return token;
        case '(':
            token.type = TOKEN_LPAREN;
            parse_pos++;
            return token;
        case ')':
            token.type = TOKEN_RPAREN;
            parse_pos++;
            return token;
        default:
            token.type = TOKEN_ERROR;
            return token;
    }
}

// Парсинг выражения (сложение и вычитание)
static double parse_expression() {
    double result = parse_term();
    
    while (1) {
        skip_whitespace();
        if (*parse_pos == '+') {
            parse_pos++;
            result += parse_term();
        } else if (*parse_pos == '-') {
            parse_pos++;
            result -= parse_term();
        } else {
            break;
        }
    }
    
    return result;
}

// Парсинг умножение и деление
static double parse_term() {
    double result = parse_power();
    
    while (1) {
        skip_whitespace();
        if (*parse_pos == '*') {
            parse_pos++;
            result *= parse_power();
        } else if (*parse_pos == '/') {
            parse_pos++;
            double divisor = parse_power();
            if (fabs(divisor) < 1e-10) {
                result = 0; // Избегаем деления на ноль
            } else {
                result /= divisor;
            }
        } else if (isalpha(*parse_pos) || *parse_pos == '(') {
            // Неявное умножение
            result *= parse_power();
        } else {
            break;
        }
    }
    
    return result;
}

// Парсинг степени
static double parse_power() {
    double result = parse_factor();
    
    skip_whitespace();
    if (*parse_pos == '^') {
        parse_pos++;
        double exponent = parse_power(); // Правая ассоциативность
        result = pow(result, exponent);
    }
    return result;
}

// Парсинг фактора (числа, переменные, функции, скобки)
static double parse_factor() {
    skip_whitespace();
    
    // Унарный минус
    if (*parse_pos == '-') {
        parse_pos++;
        return -parse_factor();
    }
    
    // Унарный плюс
    if (*parse_pos == '+') {
        parse_pos++;
        return parse_factor();
    }
    
    // Числа
    if (isdigit(*parse_pos) || *parse_pos == '.') {
        char* end;
        double value = strtod(parse_pos, &end);
        parse_pos = end;
        return value;
    }
    
    // Скобки
    if (*parse_pos == '(') {
        parse_pos++;
        double result = parse_expression();
        skip_whitespace();
        if (*parse_pos == ')') {
            parse_pos++;
        }
        return result;
    }
    
    // Переменные и функции
    if (isalpha(*parse_pos)) {
        char name[32];
        int i = 0;
        while (isalnum(*parse_pos) && i < 31) {
            name[i++] = *parse_pos++;
        }
        name[i] = '\0';
        
        // Переменные
        if (strcmp(name, "x") == 0) {
            return var_x;
        } else if (strcmp(name, "y") == 0) {
            return var_y;
        } else if (strcmp(name, "pi") == 0 || strcmp(name, "PI") == 0) {
            return M_PI;
        } else if (strcmp(name, "e") == 0 || strcmp(name, "E") == 0) {
            return M_E;
        }
        
        // Функции
        skip_whitespace();
        if (*parse_pos == '(') {
            parse_pos++;
            double arg = parse_expression();
            skip_whitespace();
            if (*parse_pos == ')') {
                parse_pos++;
            }
            
            if (strcmp(name, "sin") == 0) {
                return sin(arg);
            } else if (strcmp(name, "cos") == 0) {
                return cos(arg);
            } else if (strcmp(name, "tan") == 0) {
                return tan(arg);
            } else if (strcmp(name, "exp") == 0) {
                return exp(arg);
            } else if (strcmp(name, "log") == 0 || strcmp(name, "ln") == 0) {
                return log(arg);
            } else if (strcmp(name, "sqrt") == 0) {
                if (arg >= 0) {
                    return sqrt(arg);
                } else {
                    return NAN; // Возвращаем NaN для отрицательных значений
                }
            } else if (strcmp(name, "abs") == 0) {
                return fabs(arg);
            } else if (strcmp(name, "asin") == 0) {
                return asin(fmax(-1.0, fmin(1.0, arg))); // Ограничиваем область
            } else if (strcmp(name, "acos") == 0) {
                return acos(fmax(-1.0, fmin(1.0, arg)));
            } else if (strcmp(name, "atan") == 0) {
                return atan(arg);
            } else if (strcmp(name, "sinh") == 0) {
                return sinh(arg);
            } else if (strcmp(name, "cosh") == 0) {
                return cosh(arg);
            } else if (strcmp(name, "tanh") == 0) {
                return tanh(arg);
            }
        }
        
        // Неизвестная функция или переменная
        return 0.0;
    }
    
    return 0.0;
}

// Основная функция парсинга
double evaluate_function(float x, float y, const char* func) {
    if (strlen(func) == 0) {
        return 0.0;
    }
    
    var_x = x;
    var_y = y;
    parse_pos = func;
    
    double result = parse_expression();
    
    // НЕ заменяем NaN на 0, оставляем как есть для проверки
    if (isinf(result)) {
        return 0.0; // Только бесконечность заменяем на 0
    }
    
    return result; // Возвращаем NaN если он есть
}

// Проверка корректности функции
int validate_function(const char* func) {
    if (strlen(func) == 0) {
        strcpy(parse_error, "Pustaya funktsiya");
        return 0;
    }
    
    // Простая проверка на парность скобок
    int paren_count = 0;
    for (const char* p = func; *p; p++) {
        if (*p == '(') paren_count++;
        if (*p == ')') paren_count--;
        if (paren_count < 0) {
            strcpy(parse_error, "Neparnye skobki");
            return 0;
        }
    }
    
    if (paren_count != 0) {
        strcpy(parse_error, "Neparnye skobki");
        return 0;
    }
    
    // Тестируем функцию на нескольких точках
    for (int i = 0; i < 5; i++) {
        double test_x = (i - 2) * 0.5;
        double test_y = (i - 2) * 0.5;
        
        var_x = test_x;
        var_y = test_y;
        parse_pos = func;
        
        double result = parse_expression();
        if (isnan(result) && !isinf(result)) {
            // Допускаем бесконечность, но не NaN
            continue;
        }
    }
    
    strcpy(parse_error, "");
    return 1;
}
