#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h> //new

#include "math_parser.h"
#include "bitmap_font.h"

#include "benchmark.h"
#include "z_cache.h"

// Глобальные переменные (добавь к другим глобальным переменным)
int optimization_enabled = 1; // 1 - включено, 0 - выключено

// Объявления функций отрисовки которые используются в benchmark.c
void draw_rect(float x, float y, float width, float height, float r, float g, float b);
void draw_text_bitmap(float x, float y, const char* text, float scale);

// Масштаб функции
float scale_factor = 0.5f;


void error_callback(int error, const char* description) {
    fputs(description, stderr); //Выводит описание ошибки GLFW в stderr
}

#define WIDTH 800
#define HEIGHT 600
#define MAX_FUNCTION_LENGTH 256


// Глобальные переменные (в начале файла)
Benchmark benchmark = {0, 0, 0, ""};
ZCache z_cache;



// Состояния приложения
typedef enum {
    STATE_MENU,
    STATE_VISUALIZATION
} AppState;

// Глобальные переменные
AppState current_state = STATE_MENU;
char user_function[MAX_FUNCTION_LENGTH] = "";
int cursor_pos = 0;
int function_ready = 0;

// Сферические координаты камеры
float cam_theta = 0.0f;
float cam_phi = 45.0f;
float cam_radius = 15.0f;

// Структура для кнопки
typedef struct {
    float x, y, width, height;
    int hovered;
    int clicked;
} Button;

Button start_button = {WIDTH/2 - 50, HEIGHT/2 - 60, 100, 40, 0, 0};

// Проверка попадания мыши в кнопку
int point_in_button(Button* btn, double x, double y) {
    return (x >= btn->x && x <= btn->x + btn->width && 
            y >= btn->y && y <= btn->y + btn->height);
}

// Обработка мыши
void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    if (current_state != STATE_MENU) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        ypos = HEIGHT - ypos; // Инвертируем Y координату
        
        if (point_in_button(&start_button, xpos, ypos) && function_ready) {
            if (validate_function(user_function)) {
                current_state = STATE_VISUALIZATION;
            }
        }
    }
}

// Обработка движения мыши
void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
    if (current_state != STATE_MENU) return;
    
    ypos = HEIGHT - ypos; // Инвертируем Y координату
    start_button.hovered = point_in_button(&start_button, xpos, ypos);
}

// Обработка колеса мыши
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (current_state == STATE_VISUALIZATION) {
        cam_radius -= yoffset * 1.0f; // Изменяем радиус камеры
        
        // Ограничиваем радиус камеры
        if (cam_radius < 3.0f) cam_radius = 3.0f;
        if (cam_radius > 50.0f) cam_radius = 50.0f;
    }
}


// Обработка клавиатуры для меню
void char_callback(GLFWwindow* window, unsigned int codepoint) {
    if (current_state != STATE_MENU) return;
    
    if (cursor_pos < MAX_FUNCTION_LENGTH - 1) {
        user_function[cursor_pos] = (char)codepoint;
        cursor_pos++;
        user_function[cursor_pos] = '\0';
        function_ready = (strlen(user_function) > 0);
        
        // Очищаем ошибку при вводе
        strcpy(parse_error, "");
    }
}

// Обработка нажатий клавиш
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (current_state == STATE_MENU) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == GLFW_KEY_BACKSPACE && cursor_pos > 0) {
                cursor_pos--;
                user_function[cursor_pos] = '\0';
                function_ready = (strlen(user_function) > 0);
                strcpy(parse_error, "");
            } else if (key == GLFW_KEY_ENTER && function_ready) {
                if (validate_function(user_function)) {
                    current_state = STATE_VISUALIZATION;
                }
            } else if (key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
    } else if (current_state == STATE_VISUALIZATION) {
        const float angle_step = 2.0f;
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            if (key == GLFW_KEY_LEFT) {
                cam_theta -= angle_step;
            } else if (key == GLFW_KEY_RIGHT) {
                cam_theta += angle_step;
            } else if (key == GLFW_KEY_UP) {
                cam_phi -= angle_step;
                if (cam_phi < 5.0f) cam_phi = 5.0f;
            } else if (key == GLFW_KEY_DOWN) {
                cam_phi += angle_step;
                if (cam_phi > 175.0f) cam_phi = 175.0f;
            } else if (key == GLFW_KEY_1) {
                scale_factor -= 0.1f; // Уменьшаем масштаб
                if (scale_factor < 0.1f) scale_factor = 0.1f; // Минимальный масштаб
            } else if (key == GLFW_KEY_2) {
                scale_factor += 0.1f; // Увеличиваем масштаб
                if (scale_factor > 3.0f) scale_factor = 3.0f; // Максимальный масштаб
            } else if (key == GLFW_KEY_ESCAPE) {
                current_state = STATE_MENU;
            }else if (key == GLFW_KEY_O) {
                optimization_enabled = !optimization_enabled;
                printf("Оптимизация %s\n", optimization_enabled ? "ВКЛЮЧЕНА" : "ВЫКЛЮЧЕНА");
                
                // При выключении оптимизации освобождаем кэш
                if (!optimization_enabled) {
                    zcache_free(&z_cache);
                }
            }
        }
    }
}
// Отрисовка прямоугольника
void draw_rect(float x, float y, float width, float height, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

// Отрисовка контура прямоугольника
void draw_rect_outline(float x, float y, float width, float height, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

// Отрисовка меню
void draw_menu() {
    //матрицы используются для преобразования координат объектов в экранные координаты
    // Переключаемся в 2D режим
    glMatrixMode(GL_PROJECTION);//матрица проекции отвечается за преобразование 3D-координат в 2D-координаты для отображения на экран
    glPushMatrix();
    glLoadIdentity(); //сбрасывает все преобразования
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);//отображение 2D объектов в 3D без учета перспективы
    glMatrixMode(GL_MODELVIEW);//переключается на матрицу модели-вида (определяет положение объектов).
    glPushMatrix();
    glLoadIdentity();
    
    glDisable(GL_DEPTH_TEST);//отключаем глубину
    
    // Заголовок
    glColor3f(0.2f, 0.2f, 0.8f);
    draw_text_bitmap(WIDTH/2 - 195, HEIGHT/2 + 100, "3D Function Visualizer", 2.0f);
    
    // Поле ввода
    float input_x = WIDTH/2 - 200;
    float input_y = HEIGHT/2 + 20;
    float input_width = 400;
    float input_height = 40;
    
    // Фон поля ввода
    draw_rect(input_x, input_y, input_width, input_height, 1.0f, 1.0f, 1.0f);
    draw_rect_outline(input_x, input_y, input_width, input_height, 0.0f, 0.0f, 0.0f);
    
    // Отображение введенного текста
    glColor3f(0.0f, 0.0f, 0.0f);
    draw_text_bitmap(input_x + 10, input_y + 15, user_function, 1.5f);
    
    // Курсор
    if ((int)(glfwGetTime() * 2) % 2) { // Мигающий курсор
        float cursor_x = input_x + 10 + strlen(user_function) * 14;//14 пикселей на исмвол
        draw_rect(cursor_x, input_y + 10, 2, 20, 0.0f, 0.0f, 0.0f);
    }
    
    // Сообщение об ошибке
    if (strlen(parse_error) > 0) {
        glColor3f(1.0f, 0.0f, 0.0f);
        draw_text_bitmap(input_x, input_y - 30, parse_error, 1.0f);
    }
    
    // Кнопка Start (перемещена ниже)
    start_button.x = WIDTH/2 - 50;
    start_button.y = HEIGHT/2 - 60; // Было HEIGHT/2 + 100, стало HEIGHT/2 - 60
    
    if (function_ready) {
        if (start_button.hovered) {
            draw_rect(start_button.x, start_button.y, start_button.width, start_button.height, 0.0f, 0.8f, 0.0f);
        } else {
            draw_rect(start_button.x, start_button.y, start_button.width, start_button.height, 0.0f, 0.6f, 0.0f);
        }
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        draw_rect(start_button.x, start_button.y, start_button.width, start_button.height, 0.5f, 0.5f, 0.5f);
        glColor3f(0.8f, 0.8f, 0.8f);
    }
    
    draw_text_bitmap(start_button.x + 20, start_button.y + 15, "START", 1.5f);
    
    glEnable(GL_DEPTH_TEST);
    
    // Возвращаемся в 3D режим
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void draw_axes() {
    glLineWidth(3.0f);
    
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-5.0, 0.0, 0.0);
    glVertex3f(5.0, 0.0, 0.0);
    glEnd();
    
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, -5.0, 0.0);
    glVertex3f(0.0, 5.0, 0.0);
    glEnd();
    
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 5.0);
    glEnd();
    
    glLineWidth(1.0f);
}

void draw_function() {
    double start_calc_time = get_current_time_ms();
    
    // Определяем область рисования
    float x_min = -5.0f, x_max = 5.0f;
    float y_min = -5.0f, y_max = 5.0f;
    float step = 0.2f;
    
    if (strstr(user_function, "sqrt(1-x^2-y^2)") != NULL) {
        x_min = -1.2f; x_max = 1.2f;
        y_min = -1.2f; y_max = 1.2f;
        step = 0.05f;
    }
    
    // Создаем/обновляем кэш если нужно и оптимизация включена
    if (optimization_enabled && (!zcache_is_valid(&z_cache, x_min, x_max, y_min, y_max, step, user_function))) {
        zcache_free(&z_cache);
        
        z_cache.width = (int)((x_max - x_min) / step) + 2;
        z_cache.height = (int)((y_max - y_min) / step) + 2;
        z_cache.x_min = x_min;
        z_cache.x_max = x_max;
        z_cache.y_min = y_min;
        z_cache.y_max = y_max;
        z_cache.step = step;
        strcpy(z_cache.last_function, user_function);
        
        // Выделяем память для кэша
        z_cache.data = (float**)malloc(z_cache.height * sizeof(float*));
        for (int i = 0; i < z_cache.height; i++) {
            z_cache.data[i] = (float*)malloc(z_cache.width * sizeof(float));
            for (int j = 0; j < z_cache.width; j++) {
                z_cache.data[i][j] = NAN;
            }
        }
    }
    
    // Подсчет точек
    int point_count = 0;
    for (float x = x_min; x < x_max; x += step) {
        for (float y = y_min; y < y_max; y += step) {
            point_count += 4;
        }
    }
    
    double end_calc_time = get_current_time_ms();
    benchmark.calculation_time = end_calc_time - start_calc_time;
    benchmark.point_count = point_count;
    
    // ОТРИСОВКА с использованием кэша или без
    double start_render_time = get_current_time_ms();
    
    glBegin(GL_TRIANGLES);
    for (float x = x_min; x < x_max; x += step) {
        for (float y = y_min; y < y_max; y += step) {
            float z1, z2, z3, z4;
            
            if (optimization_enabled) {
                // Используем кэшированные значения
                z1 = zcache_get(&z_cache, x, y, user_function, scale_factor);
                z2 = zcache_get(&z_cache, x + step, y, user_function, scale_factor);
                z3 = zcache_get(&z_cache, x, y + step, user_function, scale_factor);
                z4 = zcache_get(&z_cache, x + step, y + step, user_function, scale_factor);
            } else {
                // Вычисляем напрямую
                z1 = evaluate_function(x, y, user_function) * scale_factor;
                z2 = evaluate_function(x + step, y, user_function) * scale_factor;
                z3 = evaluate_function(x, y + step, user_function) * scale_factor;
                z4 = evaluate_function(x + step, y + step, user_function) * scale_factor;
            }

            // Заменяем NaN на 0 вместо пропуска треугольника
            if (isnan(z1)) z1 = 0.0f;
            if (isnan(z2)) z2 = 0.0f;
            if (isnan(z3)) z3 = 0.0f;
            if (isnan(z4)) z4 = 0.0f;

            // Ограничиваем значения Z для стабильности визуализации
            z1 = fmax(-10.0, fmin(10.0, z1));
            z2 = fmax(-10.0, fmin(10.0, z2));
            z3 = fmax(-10.0, fmin(10.0, z3));
            z4 = fmax(-10.0, fmin(10.0, z4));

            // Адаптивное ограничение для разных функций
            float limit = 10.0f;
            if (strstr(user_function, "abs(x)") != NULL || strstr(user_function, "abs(y)") != NULL) {
                limit = 15.0f; // Больший лимит для функций с abs
            }

            z1 = fmax(-limit, fmin(limit, z1));
            z2 = fmax(-limit, fmin(limit, z2));
            z3 = fmax(-limit, fmin(limit, z3));
            z4 = fmax(-limit, fmin(limit, z4));

            // Нормализация с учетом нового лимита
            float color1 = (z1 + limit) / (2.0f * limit);
            float color2 = (z2 + limit) / (2.0f * limit);
            float color3 = (z3 + limit) / (2.0f * limit);
            float color4 = (z4 + limit) / (2.0f * limit);

            // Первый треугольник
            if (fabs(z1) < 0.01f) glColor3f(0.7f, 0.7f, 0.7f); else glColor3f(color1, 1.0f - color1, 0.0f);
            glVertex3f(x, y, z1);
            if (fabs(z2) < 0.01f) glColor3f(0.7f, 0.7f, 0.7f); else glColor3f(color2, 1.0f - color2, 0.0f);
            glVertex3f(x + step, y, z2);
            if (fabs(z3) < 0.01f) glColor3f(0.7f, 0.7f, 0.7f); else glColor3f(color3, 1.0f - color3, 0.0f);
            glVertex3f(x, y + step, z3);

            // Второй треугольник
            if (fabs(z2) < 0.01f) glColor3f(0.7f, 0.7f, 0.7f); else glColor3f(color2, 1.0f - color2, 0.0f);
            glVertex3f(x + step, y, z2);
            if (fabs(z4) < 0.01f) glColor3f(0.7f, 0.7f, 0.7f); else glColor3f(color4, 1.0f - color4, 0.0f);
            glVertex3f(x + step, y + step, z4);
            if (fabs(z3) < 0.01f) glColor3f(0.7f, 0.7f, 0.7f); else glColor3f(color3, 1.0f - color3, 0.0f);
            glVertex3f(x, y + step, z3);
        }
    }
    glEnd();
    
    double end_render_time = get_current_time_ms();
    benchmark.rendering_time = end_render_time - start_render_time;
    
    // Проверяем, изменилась ли функция
    if (strcmp(benchmark.last_function, user_function) != 0) {
        strcpy(benchmark.last_function, user_function);
        print_benchmark_results(&benchmark);
    }
}



void draw_grid() {
    glColor3f(0.3, 0.3, 0.3);
    glBegin(GL_LINES);
    for (float i = -5.0; i <= 5.0; i += 1.0) {
        glVertex3f(-5.0, i, 0.0);
        glVertex3f(5.0, i, 0.0);
        glVertex3f(i, -5.0, 0.0);
        glVertex3f(i, 5.0, 0.0);
    }
    glEnd();
}
    
int main(void) {
    //инициализация GLFW 
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        return -1;
    }
    
    //создание окна
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "3D Function Visualizer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window); //работаем с этим окном, Все последующие OpenGL команды будут применяться к этому окну.
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetScrollCallback(window, scroll_callback);


    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

    printf("3D Function Visualizer с полным парсером математических выражений\n");
    printf("=================================================================\n");
    printf("Поддерживаемые функции:\n");
    printf("  Тригонометрические: sin, cos, tan, asin, acos, atan\n");
    printf("  Гиперболические: sinh, cosh, tanh\n");
    printf("  Экспоненциальные: exp, log, ln, sqrt\n");
    printf("  Другие: abs\n");
    printf("Константы: pi, e\n");
    printf("Операторы: +, -, *, /, ^ (степень)\n");
    printf("Переменные: x, y\n");
    printf("Примеры функций:\n");
    printf("  sin(x*y) - синус произведения\n");
    printf("  x^2+y^2 - параболоид\n");
    printf("  x^2-y^2 - седло\n");
    printf("  sin(sqrt(x^2+y^2)) - волны от центра\n");
    printf("  exp(-(x^2+y^2)) - гауссиан\n");
    printf("  cos(x)+sin(y) - волнистая поверхность\n");
    printf("  abs(x)+abs(y) - пирамида\n");
    printf("  sin(x)*cos(y) - рябь\n");
    printf("\nУправление:\n");
    printf("В меню: введите функцию и нажмите START или Enter\n");
    printf("В режиме визуализации: стрелки - вращение камеры, ESC - возврат в меню\n");

    init_benchmark();
    zcache_init(&z_cache);

    while (!glfwWindowShouldClose(window)) {
        double current_time = get_current_time_seconds();
        
        // Расчет FPS с усреднением
        frame_count++;
        
        // Сохраняем время кадра для усреднения
        static double last_frame_time = 0.0;
        double frame_time = current_time - last_frame_time;
        last_frame_time = current_time;
        
        frame_times[frame_time_index] = frame_time;
        frame_time_index = (frame_time_index + 1) % 60;
        
        // Обновляем FPS каждую секунду с усреднением
        if (current_time - last_fps_time >= 0.5) { // Чаще обновляем, но усредняем
            double avg_frame_time = 0.0;
            for (int i = 0; i < 60; i++) {
                avg_frame_time += frame_times[i];
            }
            avg_frame_time /= 60;
            
            if (avg_frame_time > 0.0) {
                fps = 1.0 / avg_frame_time;
            }
            
            frame_count = 0;
            last_fps_time = current_time;
        }


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (current_state == STATE_MENU) {
            draw_menu();
        } else if (current_state == STATE_VISUALIZATION) {
            // Настройка 3D проекции
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(45.0, (double)WIDTH / (double)HEIGHT, 0.1, 100.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            // Преобразуем сферические координаты в декартовы для позиции камеры
            float theta_rad = cam_theta * M_PI / 180.0f;
            float phi_rad = cam_phi * M_PI / 180.0f;
            
            float cam_x = cam_radius * sin(phi_rad) * cos(theta_rad);
            float cam_y = cam_radius * sin(phi_rad) * sin(theta_rad);
            float cam_z = cam_radius * cos(phi_rad);

            // Устанавливаем камеру используя сферические координаты
            gluLookAt(cam_x, cam_y, cam_z,  // позиция камеры
                    0.0, 0.0, 0.0,        // точка, на которую смотрим
                    0.0, 0.0, 1.0);       // вектор "вверх"

            // Рисуем сетку в основании
            draw_grid();
            
            // Рисуем оси координат
            draw_axes();
            
            // Рисуем функцию
            draw_function();

            // Отрисовываем FPS в углу экрана
            draw_fps_display();

            // Обновляем заголовок окна с информацией о функции
            char title[512];
            snprintf(title, sizeof(title), "3D Function Visualizer - f(x,y) = %s", user_function);
            glfwSetWindowTitle(window, title);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();//быстренько проверяет события и продолжает
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    zcache_free(&z_cache);
    return 0;
}
