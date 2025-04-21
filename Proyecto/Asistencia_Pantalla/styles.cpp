#include "styles.h"

lv_style_t estilo_label;
lv_style_t estilo_boton;
lv_style_t estilo_barra_tabs;
lv_style_t estilo_btn_register; // <-- definición
lv_style_t estilo_btn_clear;    // <-- definición

void inicializar_estilos() {
  // Estilo para labels
  lv_style_init(&estilo_label);
  lv_style_set_text_font(&estilo_label, &lv_font_montserrat_16);
  lv_style_set_text_color(&estilo_label, lv_color_black());

  // Estilo genérico para botones
  lv_style_init(&estilo_boton);
  lv_style_set_radius(&estilo_boton, 10);
  lv_style_set_bg_color(&estilo_boton, lv_color_hex(0x1976D2));
  lv_style_set_text_color(&estilo_boton, lv_color_white());
  lv_style_set_border_width(&estilo_boton, 2);
  lv_style_set_border_color(&estilo_boton, lv_color_hex(0x0D47A1));

  // Estilo para barra de tabs (sin cambios)
  lv_style_init(&estilo_barra_tabs);
  lv_style_set_pad_all(&estilo_barra_tabs, 0);
  lv_style_set_text_font(&estilo_barra_tabs, &lv_font_montserrat_12);
  lv_style_set_border_width(&estilo_barra_tabs, 0);
  lv_style_set_bg_opa(&estilo_barra_tabs, LV_OPA_TRANSP);

  // Estilo específico para botón REGISTRAR (azul sólido)
  lv_style_init(&estilo_btn_register);
  lv_style_set_bg_color(&estilo_btn_register, lv_color_hex(0x0066CC));
  lv_style_set_text_color(&estilo_btn_register, lv_color_white());
  lv_style_set_radius(&estilo_btn_register, 10);
  lv_style_set_border_width(&estilo_btn_register, 0);

  // Estilo específico para botón LIMPIAR (rojo)
  lv_style_init(&estilo_btn_clear);
  lv_style_set_bg_color(&estilo_btn_clear, lv_color_hex(0xF44336));
  lv_style_set_text_color(&estilo_btn_clear, lv_color_white());
  lv_style_set_radius(&estilo_btn_clear, 10);
  lv_style_set_border_width(&estilo_btn_clear, 0);
}
