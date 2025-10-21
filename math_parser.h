#ifndef MATH_PARSER_H
#define MATH_PARSER_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// Типы токенов для парсера
typedef enum {
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_FUNCTION,
    TOKEN_OPERATOR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_END,
    TOKEN_ERROR
} TokenType;

// Структура токена
typedef struct {
    TokenType type;
    double value;
    char text[32];
} Token;

// Основные функции парсера
double evaluate_function(float x, float y, const char* func);
int validate_function(const char* func);

// Глобальная переменная для хранения ошибки парсинга
extern char parse_error[256];

#endif // MATH_PARSER_H
