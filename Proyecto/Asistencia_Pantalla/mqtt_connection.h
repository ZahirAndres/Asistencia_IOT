#ifndef MQTT_CONNECTION_H
#define MQTT_CONNECTION_H

#include <WiFi.h>
#include <PubSubClient.h>

// Credenciales
extern const char* ssid;
extern const char* password;

// MQTT
extern const char* mqtt_server;
extern const int   mqtt_port;
extern const char* mqtt_mensaje;
extern const char* mqtt_result;
extern const char* mqtt_registrar;
extern const char* mqtt_form;
extern const char* mqtt_id;
extern const char* topic_asis_codigo;


extern PubSubClient client;

// Setup y reconexión
void setup_wifi();
void reconnect_mqtt();

// Callback de recepción
void mqtt_callback(char* topic, byte* payload, unsigned int length);

#endif 