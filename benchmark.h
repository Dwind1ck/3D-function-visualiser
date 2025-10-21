#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>

#define MAX_FUNCTION_LENGTH 256

typedef struct {
    double calculation_time;
    double rendering_time;
    int point_count;
    char last_function[MAX_FUNCTION_LENGTH];
} Benchmark;

// Объявляем глобальные переменные как extern
extern double fps;
extern double last_fps_time;
extern int frame_count;
extern double frame_times[60];
extern int frame_time_index;
extern int optimization_enabled; // Только объявление

// Функции бенчмарка
void init_benchmark();
double get_current_time_ms();
double get_current_time_seconds();
void update_fps();
void print_benchmark_results(Benchmark* benchmark);
void draw_fps_display();

// Объявляем функции отрисовки которые используются в benchmark.c
void draw_rect(float x, float y, float width, float height, float r, float g, float b);
void draw_text_bitmap(float x, float y, const char* text, float scale);

#endif