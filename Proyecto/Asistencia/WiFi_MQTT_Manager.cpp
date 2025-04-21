#include "WiFi_MQTT_Manager.h"
#include "BuzzerManager.h"

const char* ssid = "MrMexico2014"; //MrMexico2014
const char* password = "Mr2014.."; //Mr2014..
const char* mqtt_server = "192.168.0.4";
const int mqtt_port = 1883;

const char* topic_add = "api/huella/add";
const char* topic_delete = "api/huella/delete";
const char* topic_update = "api/huella/update";
const char* topic_store = "api/huella/store";
const char* topic_login = "api/huella/login";
const char* topic_delete_result = "api/huella/delete/result";
const char* topic_update_result = "api/huella/update/result";
const char* topic_result = "api/result";
const char* topic_asis_huella = "api/huella";
const char* topic_asis_rfid = "api/rfid";

WiFiClient espClient;
PubSubClient client(espClient);

// Variables globales para compartir el estado
String operation = "verify";
int targetID = -1;
int idUsuarioDB = -1;

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  playSuccessMelody();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.print("Error al parsear JSON: ");
    Serial.println(error.c_str());
    return;
  }

  targetID = doc["idHuella"];
  idUsuarioDB = doc["idUsuario"];

  if (strcmp(topic, topic_add) == 0)
    operation = "add";
  else if (strcmp(topic, topic_delete) == 0)
    operation = "delete";
  else if (strcmp(topic, topic_update) == 0)
    operation = "update";

  Serial.printf("Operación: %s | ID Huella: %d | ID Usuario: %d\n", 
                operation.c_str(), targetID, idUsuarioDB);
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando a MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado a MQTT");
      client.subscribe(topic_add);
      client.subscribe(topic_delete);
      client.subscribe(topic_update);
    } else {
      Serial.print("Error: ");
      Serial.println(client.state());
      beep();
      delay(2000);
    }
  }
}
