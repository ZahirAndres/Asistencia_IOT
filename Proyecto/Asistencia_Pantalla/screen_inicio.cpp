#include "screen_inicio.h"
#include "styles.h"
#include "mqtt_connection.h"  // Asegúrate de tener tu cliente MQTT aquí
#include <ArduinoJson.h>      // Para enviar JSON
#include "screen_teclado.h"
#include "screen_manager.h"

static lv_obj_t* label_mensaje;
static lv_obj_t* label_result;
static lv_obj_t* label_identificacion;
static lv_obj_t* input_identificacion;
static lv_obj_t* btn_enviar;
static lv_obj_t* btn_asistencia;

void screen_inicio_set_mensaje_vaciar() {
  lv_label_set_text(label_mensaje, "");
}


static void input_identificacion_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_FOCUSED) {
    lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);           // Mostrar teclado
    lv_keyboard_set_textarea(keyboard, input_identificacion);  // Enlazar teclado al input
  } else if (code == LV_EVENT_DEFOCUSED) {
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);  // Ocultar teclado
  }
}

void enviar_identificacion_click(lv_event_t* e) {
  const char* identificacion = lv_textarea_get_text(input_identificacion);

  if (strlen(identificacion) == 0) {
    Serial.println("Identificación vacía, no se enviará.");
    return;
  }

  StaticJsonDocument<128> doc;
  doc["tipo"] = "codigo";
  doc["valor"] = identificacion;

  char buffer[128];
  serializeJson(doc, buffer);

  if (client.connected()) {
    client.publish(mqtt_id, buffer);
    Serial.print("Identificación enviada por MQTT: ");
    Serial.println(buffer);
  } else {
    Serial.println("MQTT no conectado, no se pudo enviar la identificación.");
  }

  // Ocultar teclado después de enviar
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);

  // Limpiar el input después de enviar
  lv_textarea_set_text(input_identificacion, "");
}

void enviar_asistencia_click(lv_event_t* e) {
  const char* identificacion = lv_textarea_get_text(input_identificacion);

  if (strlen(identificacion) == 0) {
    Serial.println("Identificación vacía, no se enviará.");
    return;
  }

  StaticJsonDocument<128> doc;
  doc["tipo"] = "identificacion";
  doc["valor"] = identificacion;

  char buffer[128];
  serializeJson(doc, buffer);

  if (client.connected()) {
    client.publish(topic_asis_codigo, buffer);
    Serial.print("Identificación enviada por MQTT: ");
    Serial.println(buffer);
  } else {
    Serial.println("MQTT no conectado, no se pudo enviar la identificación.");
  }

  // Ocultar teclado después de enviar
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);

  // Limpiar el input después de enviar
  lv_textarea_set_text(input_identificacion, "");
}


void screen_inicio_create(lv_obj_t* parent) {
  // Label para el mensaje MQTT
  label_mensaje = lv_label_create(parent);
  lv_obj_set_width(label_mensaje, 200);
  lv_obj_add_style(label_mensaje, &estilo_label, 0);
  lv_label_set_long_mode(label_mensaje, LV_LABEL_LONG_WRAP);
  lv_label_set_text(label_mensaje, "...");
  lv_obj_align(label_mensaje, LV_ALIGN_TOP_MID, 0, 20);

  // Label para el resultado MQTT
  label_result = lv_label_create(parent);
  lv_obj_set_width(label_result, 200);
  lv_obj_add_style(label_result, &estilo_label, 0);
  lv_label_set_long_mode(label_result, LV_LABEL_LONG_WRAP);
  lv_label_set_text(label_result, "Indicaciones...");
  lv_obj_align(label_result, LV_ALIGN_TOP_MID, 0, 70);

  // NUEVO: Label para Identificación
  label_identificacion = lv_label_create(parent);
  lv_label_set_text(label_identificacion, "Identificación:");
  lv_obj_add_style(label_identificacion, &estilo_label, 0);
  lv_obj_align(label_identificacion, LV_ALIGN_TOP_MID, 0, 130);

  // NUEVO: Input de Identificación
  input_identificacion = lv_textarea_create(parent);
  lv_textarea_set_one_line(input_identificacion, true);
  lv_obj_set_size(input_identificacion, 160, 30);
  lv_obj_align(input_identificacion, LV_ALIGN_TOP_MID, 0, 160);
  lv_obj_add_style(input_identificacion, &estilo_boton, LV_PART_MAIN);
  lv_obj_add_event_cb(input_identificacion, input_identificacion_event_cb, LV_EVENT_ALL, NULL);


  // NUEVO: Botón para enviar identificación
  btn_enviar = lv_btn_create(parent);
  lv_obj_set_size(btn_enviar, 160, 40);
  lv_obj_align(btn_enviar, LV_ALIGN_TOP_LEFT, 0, 200);
  lv_obj_add_style(btn_enviar, &estilo_btn_register, LV_PART_MAIN);

  lv_obj_t* label_btn = lv_label_create(btn_enviar);
  lv_label_set_text(label_btn, "Agregar huella y rfid");
  lv_obj_center(label_btn);
  lv_obj_add_event_cb(btn_enviar, enviar_identificacion_click, LV_EVENT_CLICKED, NULL);

  // NUEVO: Botón para enviar asistencia
  btn_asistencia = lv_btn_create(parent);
  lv_obj_set_size(btn_asistencia, 100, 40);
  lv_obj_align(btn_asistencia, LV_ALIGN_TOP_RIGHT, 0, 200);
  lv_obj_add_style(btn_asistencia, &estilo_btn_register, LV_PART_MAIN);

  lv_obj_t* label_btn_asis = lv_label_create(btn_asistencia);
  lv_label_set_text(label_btn_asis, "Asistencia");
  lv_obj_center(label_btn_asis);
  lv_obj_add_event_cb(btn_asistencia, enviar_asistencia_click, LV_EVENT_CLICKED, NULL);
}

void screen_inicio_set_mensaje(const char* msg) {
  lv_label_set_text(label_mensaje, msg);
}


void screen_inicio_set_result(const char* msg) {
  lv_label_set_text(label_result, msg);
}
