#include <Arduino.h>
#include "screen_register.h"
#include "styles.h"
#include "screen_manager.h"
#include "config.h"
#include "mqtt_connection.h"
#include <ArduinoJson.h>

extern lv_obj_t* keyboard;
static lv_obj_t* input_name;
static lv_obj_t* input_ape;
static lv_obj_t* input_codigo;
static lv_obj_t* btn_register;
static lv_obj_t* btn_clear;

static lv_obj_t* label_resultado = NULL;
void screen_register_set_result(const char* mensaje) {
  if (label_resultado != NULL) {
    lv_label_set_text(label_resultado, mensaje);
  }
}

// Función reutilizable para limpiar campos
static void limpiar_campos() {
  lv_textarea_set_text(input_name, "");
  lv_textarea_set_text(input_ape, "");
  lv_textarea_set_text(input_codigo, "");
  lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
  Serial.println("Campos limpiados.");
}

static void btn_register_click(lv_event_t* e) {
  const char* name = lv_textarea_get_text(input_name);
  const char* ape = lv_textarea_get_text(input_ape);
  const char* codigo = lv_textarea_get_text(input_codigo);

  if (strlen(name) == 0 || strlen(ape) == 0 || strlen(codigo) == 0) {
    Serial.println("Error: Todos los campos deben estar llenos.");

    // Crear JSON de error
    StaticJsonDocument<128> errorDoc;
    errorDoc["tipo"] = "error";
    errorDoc["mensaje"] = "Todos los campos deben estar llenos.";

    char errorBuffer[128];
    serializeJson(errorDoc, errorBuffer);

    if (client.connected()) {
      client.publish(mqtt_form, errorBuffer);
      Serial.print("Error enviado por MQTT: ");
      Serial.println(errorBuffer);
    } else {
      Serial.println("MQTT no conectado para enviar el error.");
    }

    return;
  }

  Serial.println("Registro iniciado");
  Serial.print("Nombre: ");
  Serial.println(name);
  Serial.print("Apellido: ");
  Serial.println(ape);
  Serial.print("Código: ");
  Serial.println(codigo);

  StaticJsonDocument<256> doc;
  doc["nombre"] = name;
  doc["apellido"] = ape;
  doc["codigo"] = codigo;

  char buffer[256];
  serializeJson(doc, buffer);

  if (client.connected()) {
    client.publish(mqtt_form, buffer);
    Serial.print("JSON enviado por MQTT: ");
    Serial.println(buffer);

    // Mensaje de ÉXITO
    StaticJsonDocument<128> successDoc;
    successDoc["tipo"] = "exito";
    successDoc["mensaje"] = "Registro enviado correctamente, compruebe su registro.";

    char successBuffer[128];
    serializeJson(successDoc, successBuffer);

    client.publish(mqtt_form, successBuffer);
    Serial.print("Mensaje de éxito enviado por MQTT: ");
    Serial.println(successBuffer);

  } else {
    Serial.println("MQTT no conectado");
  }

  limpiar_campos();
}


static void btn_clear_click(lv_event_t* e) {
  limpiar_campos();
}

// Limpieza individual por input
// En tu función limpiar_input:
static void limpiar_input(lv_event_t* e) {
  lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
  lv_obj_t* input = (lv_obj_t*)lv_event_get_user_data(e);
  lv_textarea_set_text(input, "");
  Serial.println("Input limpiado.");
}


void crear_input_con_clear(lv_obj_t* parent, const char* label_text, lv_obj_t** input_var, int y) {
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, label_text);
  lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, y);
  lv_obj_add_style(label, &estilo_label, LV_PART_MAIN);

  *input_var = lv_textarea_create(parent);
  lv_textarea_set_one_line(*input_var, true);
  lv_obj_set_size(*input_var, 160, 30);
  lv_obj_align(*input_var, LV_ALIGN_TOP_LEFT, 90, y);
  lv_obj_add_style(*input_var, &estilo_boton, LV_PART_MAIN);

  lv_obj_add_event_cb(
    *input_var, [](lv_event_t* e) {
      if (lv_event_get_code(e) == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(keyboard, (lv_obj_t*)lv_event_get_target(e));
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(keyboard);
      }
    },
    LV_EVENT_FOCUSED, NULL);


  // Botón LIMPIAR individual
  lv_obj_t* btn_clear_input = lv_btn_create(parent);
  lv_obj_set_size(btn_clear_input, 30, 30);
  lv_obj_align_to(btn_clear_input, *input_var, LV_ALIGN_OUT_RIGHT_MID, 4, 0);
  lv_obj_add_style(btn_clear_input, &estilo_btn_clear, LV_PART_MAIN);

  lv_obj_t* label_clear = lv_label_create(btn_clear_input);
  lv_label_set_text(label_clear, "X");
  lv_obj_center(label_clear);

  lv_obj_add_event_cb(btn_clear_input, limpiar_input, LV_EVENT_CLICKED, *input_var);
}

void screen_register_create(lv_obj_t* parent) {
  // Inputs con botón limpiar al lado
  crear_input_con_clear(parent, "Nombre:", &input_name, 20);
  crear_input_con_clear(parent, "Apellido:", &input_ape, 70);
  crear_input_con_clear(parent, "Código:", &input_codigo, 120);

  // Botón REGISTRAR centrado
  btn_register = lv_btn_create(parent);
  lv_obj_set_size(btn_register, 100, 40);
  lv_obj_align(btn_register, LV_ALIGN_LEFT_MID, 0, 150);  // Ajusté la alineación para no solaparse
  lv_obj_add_style(btn_register, &estilo_btn_register, LV_PART_MAIN);

  lv_obj_t* label_registrar = lv_label_create(btn_register);
  lv_label_set_text(label_registrar, "Registrar");
  lv_obj_center(label_registrar);
  lv_obj_add_event_cb(btn_register, btn_register_click, LV_EVENT_CLICKED, NULL);

  // Botón LIMPIAR centrado debajo del botón REGISTRAR
  btn_clear = lv_btn_create(parent);
  lv_obj_set_size(btn_clear, 100, 40);
  lv_obj_align(btn_clear, LV_ALIGN_RIGHT_MID, 0, 150);  // Ajusté la posición para no solaparse con el mensaje
  lv_obj_add_style(btn_clear, &estilo_btn_clear, LV_PART_MAIN);

  lv_obj_t* label_clear = lv_label_create(btn_clear);
  lv_label_set_text(label_clear, "Limpiar");
  lv_obj_center(label_clear);
  lv_obj_add_event_cb(btn_clear, btn_clear_click, LV_EVENT_CLICKED, NULL);

  // Etiqueta de resultado debajo de los botones
  label_resultado = lv_label_create(parent);
  lv_label_set_text(label_resultado, "");
  lv_obj_set_width(label_resultado, 200);                       // ← Establece el límite de ancho
  lv_label_set_long_mode(label_resultado, LV_LABEL_LONG_WRAP);  // ← Hace que el texto se ajuste al ancho
  lv_obj_align(label_resultado, LV_ALIGN_BOTTOM_MID, 0, 20);    // Posicionamiento
}
