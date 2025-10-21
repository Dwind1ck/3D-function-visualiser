#ifndef BITMAP_FONT_H
#define BITMAP_FONT_H                                 

#include <GL/gl.h>
#include <stddef.h>

// Простой растровый шрифт 5x7 пикселей
extern unsigned char font_5x7[][7];

// Функции для работы со шрифтом
int get_char_index(char c);
void draw_char_bitmap(float x, float y, char c, float scale);
void draw_text_bitmap(float x, float y, const char* text, float scale);

#endif
