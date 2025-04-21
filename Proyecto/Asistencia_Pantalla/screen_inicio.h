#ifndef SCREEN_INICIO_H
#define SCREEN_INICIO_H

#include <lvgl.h>

void screen_inicio_create(lv_obj_t* parent);
void screen_inicio_set_mensaje(const char* msg);
void screen_inicio_set_result(const char* msg);
void screen_inicio_set_mensaje_vaciar();
#endif  // SCREEN_INICIO_H