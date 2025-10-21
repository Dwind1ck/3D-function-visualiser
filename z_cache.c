#include "z_cache.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Объявляем optimization_enabled как extern
extern int optimization_enabled;

void zcache_init(ZCache* cache) {
    cache->data = NULL;
    cache->width = 0;
    cache->height = 0;
    cache->last_function[0] = '\0';
}

void zcache_free(ZCache* cache) {
    if (cache->data) {
        for (int i = 0; i < cache->height; i++) {
            free(cache->data[i]);
        }
        free(cache->data);
        cache->data = NULL;
    }
    cache->width = 0;
    cache->height = 0;
}

int zcache_is_valid(ZCache* cache, float x_min, float x_max, float y_min, float y_max, float step, const char* func) {
    return (cache->data != NULL && 
            fabs(cache->x_min - x_min) < 0.001f &&
            fabs(cache->x_max - x_max) < 0.001f &&
            fabs(cache->y_min - y_min) < 0.001f &&
            fabs(cache->y_max - y_max) < 0.001f &&
            fabs(cache->step - step) < 0.001f &&
            strcmp(cache->last_function, func) == 0);
}

// Исправленная функция - убрал лишний параметр optimization_enabled
float zcache_get(ZCache* cache, float x, float y, const char* func, float scale_factor) {
    // Если оптимизация выключена - вычисляем напрямую
    if (!optimization_enabled) {
        return evaluate_function(x, y, func) * scale_factor;
    }
    
    // Если точка вне кэша - вычисляем напрямую
    if (x < cache->x_min || x > cache->x_max || y < cache->y_min || y > cache->y_max) {
        return evaluate_function(x, y, func) * scale_factor;
    }
    
    // Вычисляем индексы в кэше
    int i = (int)((x - cache->x_min) / cache->step);
    int j = (int)((y - cache->y_min) / cache->step);
    
    if (i >= 0 && i < cache->width && j >= 0 && j < cache->height) {
        if (isnan(cache->data[j][i])) {
            cache->data[j][i] = evaluate_function(x, y, func) * scale_factor;
        }
        return cache->data[j][i];
    }
    
    return evaluate_function(x, y, func) * scale_factor;
}