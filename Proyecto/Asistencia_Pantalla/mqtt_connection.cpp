#include "mqtt_connection.h"
#include "screen_inicio.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "screen_register.h"


// WiFi
const char* ssid = "MrMexico2014";
const char* password = "Mr2014..";

// Broker MQTT
const char* mqtt_server = "192.168.0.4";
const int mqtt_port = 1883;
const char* mqtt_mensaje = "api/mensaje";
const char* mqtt_result = "api/result";
const char* mqtt_registrar = "api/pantalla/registrar";
const char* mqtt_form = "api/result/form";
const char* mqtt_id = "api/identificacion";
const char* topic_asis_codigo = "api/codigo";


WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(100);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char msg[length + 1];
  for (unsigned int i = 0; i < length; i++) msg[i] = (char)payload[i];
  msg[length] = '\0';

  const char* mensaje = nullptr;

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, msg);

  if (!error) {
    // Intentar obtener 'mensaje'
    if (doc.containsKey("mensaje")) {
      mensaje = doc["mensaje"];
    }
    // Si es un array con 'registrar_asistencia'
    else if (doc.is<JsonArray>()) {
      JsonArray array = doc.as<JsonArray>();
      if (!array.isNull() && array.size() > 0) {
        JsonObject obj = array[0];
        mensaje = obj["registrar_asistencia"] | nullptr;
      }
    }
  }

  // Si no encontró nada, usar el texto recibido tal cual
  if (mensaje == nullptr) {
    mensaje = msg;
  }

  // Mostrar en la pantalla según el topic
  if (strcmp(topic, mqtt_mensaje) == 0) {
    screen_inicio_set_mensaje(mensaje);

  } else if (strcmp(topic, mqtt_result) == 0) {
    screen_inicio_set_result(mensaje);

  } else if (strcmp(topic, mqtt_form) == 0) {
    screen_register_set_result(mensaje);
  }
}


void reconnect_mqtt() {
  while (!client.connected()) {
    if (client.connect("ESP32_Client")) {
      client.subscribe(mqtt_mensaje);
      client.subscribe(mqtt_result);
      client.subscribe(mqtt_registrar);
      client.subscribe(mqtt_form);
      client.subscribe(mqtt_id);
    } else {
      delay(5000);
    }
  }
}