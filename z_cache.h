#ifndef Z_CACHE_H
#define Z_CACHE_H

#include "math_parser.h"
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600
#define MAX_FUNCTION_LENGTH 256

// Объявляем optimization_enabled как extern
extern int optimization_enabled;

// Структура для кэша Z-значений
typedef struct {
    float** data;
    int width;
    int height;
    float x_min;
    float x_max;
    float y_min;
    float y_max;
    float step;
    char last_function[MAX_FUNCTION_LENGTH];
} ZCache;

// Функции кэширования
void zcache_init(ZCache* cache);
void zcache_free(ZCache* cache);
float zcache_get(ZCache* cache, float x, float y, const char* func, float scale_factor); // Убрал лишний параметр
int zcache_is_valid(ZCache* cache, float x_min, float x_max, float y_min, float y_max, float step, const char* func);

#endif