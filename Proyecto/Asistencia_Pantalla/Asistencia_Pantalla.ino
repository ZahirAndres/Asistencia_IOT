#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "mqtt_connection.h"
#include "screen_inicio.h"
#include "screen_register.h"
#include "screen_teclado.h"
#include "styles.h"
#include "config.h"
#include "screen_manager.h"

// Touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33



SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

lv_obj_t* tabview;

void setup() {
  Serial.begin(115200);
  lv_init();

  // Inicializa pantalla táctil SPI
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Asegúrate de usar la misma rotación que en la calibración
  touchscreen.setRotation(2);

  // Inicializa TFT para LVGL
  lv_display_t* disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

  // Configura entrada táctil para LVGL
  lv_indev_t* indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, [](lv_indev_t* indev, lv_indev_data_t* data) {
    if (touchscreen.touched()) {
      TS_Point p = touchscreen.getPoint();

      // Usa rotación configurada en touchscreen.setRotation()
      int16_t x = p.x;
      int16_t y = p.y;

      // Asegúrate de mapear según el tamaño de la pantalla y orientación
      x = map(x, 200, 3700, 0, SCREEN_WIDTH);   // Ajusta estos valores según sea necesario
      y = map(y, 240, 3800, 0, SCREEN_HEIGHT);  // Igual aquí

      x = constrain(x, 0, SCREEN_WIDTH - 1);
      y = constrain(y, 0, SCREEN_HEIGHT - 1);

      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = x;
      data->point.y = y;

      Serial.print("Mapped: (");
      Serial.print(x);
      Serial.print(",");
      Serial.print(y);
      Serial.println(")");
    } else {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  });




  inicializar_estilos();

  tabview = lv_tabview_create(lv_scr_act());

  // Obtener barra de pestañas
  lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);

  // Deshabilita el ajuste automático de tamaño
  lv_obj_set_height(tab_btns, 30);  // Aquí fuerzas una altura menor
  lv_obj_add_style(tab_btns, &estilo_barra_tabs, 0);

  lv_obj_t* tab_inicio = lv_tabview_add_tab(tabview, "Inicio");
  lv_obj_t* tab_register = lv_tabview_add_tab(tabview, "Registrarse");

  screen_inicio_create(tab_inicio);
  screen_register_create(tab_register);

  // Crea teclado flotante en la pantalla principal
  screen_teclado_create(lv_scr_act());

  // Conexión WiFi/MQTT
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);
}

void loop() {
  if (!client.connected()) reconnect_mqtt();
  client.loop();
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);
}
