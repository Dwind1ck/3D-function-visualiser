#include "benchmark.h"
#include <stdio.h>
#include <string.h>

// Глобальные переменные для FPS
double fps = 0.0;
double last_fps_time = 0.0;
int frame_count = 0;
double frame_times[60];
int frame_time_index = 0;

void init_benchmark() {
    for (int i = 0; i < 60; i++) {
        frame_times[i] = 0.0167; // Инициализируем 60 FPS
    }
}

double get_current_time_ms() {
    return (double)clock() / CLOCKS_PER_SEC * 1000.0;
}

double get_current_time_seconds() {
    return get_current_time_ms() / 1000.0;
}

void update_fps() {
    static double last_frame_time = 0.0;
    double current_time = get_current_time_seconds();
    
    if (last_frame_time > 0.0) {
        double frame_time = current_time - last_frame_time;
        
        frame_times[frame_time_index] = frame_time;
        frame_time_index = (frame_time_index + 1) % 60;
        
        // Обновляем FPS каждые 0.5 секунды с усреднением
        if (current_time - last_fps_time >= 0.5) {
            double avg_frame_time = 0.0;
            for (int i = 0; i < 60; i++) {
                avg_frame_time += frame_times[i];
            }
            avg_frame_time /= 60;
            
            if (avg_frame_time > 0.0) {
                fps = 1.0 / avg_frame_time;
            }
            last_fps_time = current_time;
        }
    }
    last_frame_time = current_time;
}

void print_benchmark_results(Benchmark* benchmark) {
    int triangles_count = benchmark->point_count / 2;
    int vertices_count = triangles_count * 3;
    int gl_calls_count = vertices_count;
    
    printf("\n=== БЕНЧМАРК ===\n");
    printf("Функция: %s\n", benchmark->last_function);
    printf("Количество точек: %d\n", benchmark->point_count);
    printf("Треугольников: %d\n", triangles_count);
    printf("Вершин: %d\n", vertices_count);
    printf("Вызовов OpenGL: ~%d\n", gl_calls_count);
    printf("Время вычисления: %.2f мс\n", benchmark->calculation_time);
    printf("Время отрисовки: %.2f мс\n", benchmark->rendering_time);
    printf("Общее время: %.2f мс\n", benchmark->calculation_time + benchmark->rendering_time);
    
    printf("\nДетальная статистика:\n");
    printf("- Вычисления: %d операций → %.2f мс\n", 
           benchmark->point_count, benchmark->calculation_time);
    printf("- Отрисовка: ~%d вызовов OpenGL → %.2f мс\n", 
           gl_calls_count, benchmark->rendering_time);
    
    if (strstr(benchmark->last_function, "sin") != NULL) {
        printf("- Сложная функция с тригонометрией\n");
    } else if (strstr(benchmark->last_function, "abs") != NULL) {
        printf("- Простая функция → быстрее вычисления!\n");
    }
    
    printf("================\n\n");
}

// Объявим эти функции как extern (они определены в main.c)
extern void draw_rect(float x, float y, float width, float height, float r, float g, float b);
extern void draw_text_bitmap(float x, float y, const char* text, float scale);

void draw_fps_display() {
    // Переключаемся в 2D режим
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1); // WIDTH, HEIGHT
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);
    
    // Рисуем FPS в правом верхнем углу
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %d", (int)fps);
    
    // Размеры текста (добавляем объявления переменных)
    float text_width = strlen(fps_text) * 12 * 1.5f;
    float text_height = 20 * 1.5f;
    float padding = 10.0f;
    
    // Черный прямоугольник под текст
    draw_rect(800 - text_width - padding * 2, 
              600 - text_height - padding * 2,
              text_width + padding * 2, 
              text_height + padding * 2,
              0.0f, 0.0f, 0.0f);
    
    // Зеленый текст поверх
    glColor3f(0.0f, 1.0f, 0.0f);
    draw_text_bitmap(800 - text_width - padding, 
                     600 - text_height - padding, 
                     fps_text, 1.5f);
    
    // Добавляем статус оптимизации
    char optim_text[32];
    snprintf(optim_text, sizeof(optim_text), "OPT: %s", optimization_enabled ? "ON" : "OFF");
    
    // Размеры текста оптимизации
    float optim_text_width = strlen(optim_text) * 12 * 1.2f;
    float optim_text_height = 20 * 1.2f;
    
    // Черный прямоугольник под текст оптимизации
    draw_rect(800 - optim_text_width - padding * 2, 
              600 - text_height - optim_text_height - padding * 4,
              optim_text_width + padding * 2, 
              optim_text_height + padding * 2,
              0.0f, 0.0f, 0.0f);
    
    // Цвет текста оптимизации (зеленый если включена, красный если выключена)
    glColor3f(optimization_enabled ? 0.0f : 1.0f, optimization_enabled ? 1.0f : 0.0f, 0.0f);
    draw_text_bitmap(800 - optim_text_width - padding, 
                     600 - text_height - optim_text_height - padding * 3, 
                     optim_text, 1.2f);
    
    glEnable(GL_DEPTH_TEST);
    
    // Возвращаемся в 3D режим
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}