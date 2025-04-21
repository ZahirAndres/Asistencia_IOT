#ifndef STYLES_H
#define STYLES_H

#include <lvgl.h>

extern lv_style_t estilo_label;
extern lv_style_t estilo_boton;        // estilo genérico, ya existente
extern lv_style_t estilo_barra_tabs;
extern lv_style_t estilo_btn_register; // <-- nuevo
extern lv_style_t estilo_btn_clear;    // <-- nuevo

void inicializar_estilos();

#endif // STYLES_H
