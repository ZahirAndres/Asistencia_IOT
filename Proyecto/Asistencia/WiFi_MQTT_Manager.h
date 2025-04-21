#ifndef WIFI_MQTT_MANAGER_H
#define WIFI_MQTT_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Variables de conexión WiFi y MQTT
extern const char* ssid;
extern const char* password;
extern const char* mqtt_server;
extern const int mqtt_port;

extern const char* topic_add;
extern const char* topic_delete;
extern const char* topic_update;
extern const char* topic_store;
extern const char* topic_login;
extern const char* topic_delete_result;
extern const char* topic_update_result;
extern const char* topic_asis_rfid;
extern const char* topic_asis_huella;

// Cliente WiFi y MQTT
extern WiFiClient espClient;
extern PubSubClient client;

// Variables para la operación actual (para utilizar en el callback y el loop)
extern String operation;
extern int targetID;
extern int idUsuarioDB;

// Funciones de conexión y callback
void connectToWiFi();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif
