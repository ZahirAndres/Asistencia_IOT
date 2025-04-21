#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>  // Para obtener fecha y hora

// Módulos personalizados
#include "WiFi_MQTT_Manager.h"    // Define connectToWiFi(), mqtt_server, mqtt_port, client, mqttCallback(), reconnectMQTT()
#include "FingerprintManager.h"   // Define finger, getFingerprintID(), enrollFingerprint(), deleteFingerprint(), operation, targetID, playSuccessMelody(), playErrorMelody(), playAlertMelody()
#include "RFID_Manager.h"         // Define leerRFID()
#include "MotionSensorManager.h"  // Define checkMotionSensor()
#include "BuzzerManager.h"        // Define initBuzzer(), beep()
#include "ClockManager.h"         // Define initClock(), updateClock()
#include "RgbLed.h"               // Define RgbLed class

// LEDs
RgbLed led1(4, LED_UNUSED, LED_UNUSED);
RgbLed led2(25, 26, LED_UNUSED);
RgbLed led3(13, 21, 27);

// Banderas de soporte de color
bool led1SoportaRGB = false;
bool led2SoportaRGB = true;
bool led3SoportaRGB = true;

// Reconexión sensor huella
bool sensorHuellaConectado = false;
unsigned long ultimoIntentoHuella = 0;
const unsigned long intervaloReintento = 3000;  // 3 segundos
unsigned long ultimoBeep = 0;
const unsigned long intervaloBeep = 800;  // 0.8 segundos

// Reporte periódico de fallo huella
unsigned long ultimoReporteHuella = 0;
const unsigned long intervaloReporteHuella = 2UL * 60UL * 60UL * 1000UL;  // 2 horas en ms

// URL de tu Google Apps Script
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbz0OfB9p3ZJN45XeOgBQRoKvoFQ0HU2WLZVWUkPui1_DJeAMzo_-NbWnzfpiKvNZrL8/exec";

// Funciones auxiliares de LEDs
void mostrarErrorSensorHuella() {
  if (led1SoportaRGB) led1.encenderAmarillo();
  else led1.encenderRojo();
  if (led2SoportaRGB) led2.encenderAmarillo();
  else led2.encenderRojo();
  led3.encenderRojo();
}

void mostrarTodoCorrecto() {
  led1.encenderVerde();
  led2.encenderVerde();
  led3.encenderVerde();
}

// Función para enviar reporte de estado del sensor de huella a Google Sheets
void reportarEstadoHuella(const char* estado) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ No se pudo reportar. WiFi desconectado.");
    return;
  }

  // Obtener fecha y hora actual vía NTP (ClockManager ya hizo initClock())
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️ No se obtuvo la hora local");
  }

  char timestamp[32];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

  // Construir JSON
  StaticJsonDocument<256> doc;
  doc["estado"] = estado;
  doc["timestamp"] = timestamp;
  String jsonData;
  serializeJson(doc, jsonData);

  // Enviar POST
  HTTPClient http;
  http.begin(googleScriptURL);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(jsonData);

  if (httpCode > 0) {
    String resp = http.getString();
    Serial.print("📤 Reporte enviado: ");
    Serial.println(resp);
  } else {
    Serial.print("❌ Error al reportar: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}

void setup() {
  Serial.begin(115200);

  // LEDs
  led1.begin();
  led2.begin();
  led3.begin();

  // Inicializar sensor huella (Serial1)
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  // Inicializar RFID
  SPI.begin();
  rfid.PCD_Init();

  // Otros módulos
  initBuzzer();
  initMotionSensor();

  // Conexión WiFi y MQTT
  connectToWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  reconnectMQTT();

  // Chequeo inicial
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ Conexión WiFi exitosa");

    // LEDs blancos de arranque
    led1.encenderBlanco();
    led2.encenderBlanco();
    led3.encenderBlanco();
    delay(1500);
    led1.apagar();
    led2.apagar();
    led3.apagar();

    // Inicializar reloj NTP
    initClock();

    // Verificar sensor de huella
    if (finger.verifyPassword()) {
      Serial.println("✅ Sensor de huella detectado");
      playSuccessMelody();
      sensorHuellaConectado = true;
    } else {
      Serial.println("❌ Sensor de huella NO detectado en setup");
      playErrorMelody();
      sensorHuellaConectado = false;
    }
  } else {
    Serial.println("❌ No conectado a WiFi en setup");
    playErrorMelody();
  }
}

void loop() {
  unsigned long ahora = millis();

  // 1) Verificar WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado, reconectando...");
    beep();
    connectToWiFi();
    led1.encenderRojo();
  }

  // 2) Reconexión sensor de huella
  bool estadoActual = finger.verifyPassword();
  if (!estadoActual && sensorHuellaConectado) {
    Serial.println("⚠️ Sensor de huella desconectado");
    led3.encenderRojo();
    sensorHuellaConectado = false;
    ultimoIntentoHuella = ahora;

    // Reporte inmediato de desconexión
    reportarEstadoHuella("DESCONECTADO");
    ultimoReporteHuella = ahora;
  }

  // 3) Si sigue desconectado, cada 2 horas reportar
  if (!sensorHuellaConectado) {
    if (ahora - ultimoReporteHuella >= intervaloReporteHuella) {
      reportarEstadoHuella("SIGUE DESCONECTADO");
      ultimoReporteHuella = ahora;
    }

    // Reintentar reconectar cada 3s
    if (ahora - ultimoIntentoHuella >= intervaloReintento) {
      Serial.println("🔄 Intentando reconectar sensor huella...");
      if (finger.verifyPassword()) {
        Serial.println("✅ Sensor de huella reconectado");
        sensorHuellaConectado = true;
        mostrarTodoCorrecto();

        // Reporte de reconexión
        reportarEstadoHuella("CONECTADO");
        ultimoReporteHuella = ahora;
      } else {
        Serial.println("❌ Aún no se detecta sensor huella");
        playAlertMelody();
        mostrarErrorSensorHuella();
      }
      ultimoIntentoHuella = ahora;
    }

    // Beep suave para indicar fallo
    if (ahora - ultimoBeep >= intervaloBeep) {
      beep();
      ultimoBeep = ahora;
    }

    return;  // Salir del loop hasta reconexión
  }

  // 4) Si todo OK, mostrar LEDs verdes
  if (sensorHuellaConectado && WiFi.status() == WL_CONNECTED && client.connected()) {
    mostrarTodoCorrecto();
  }

  // 5) Reconectar MQTT si hace falta
  if (!client.connected()) {
    Serial.println("⚠️ MQTT desconectado, reconectando...");
    beep();
    reconnectMQTT();
  }
  client.loop();

  // 6) Operaciones principales
  leerRFID();
  checkMotionSensor();
  updateClock();

  if (operation == "verify") {
    getFingerprintID();
  } else if (operation == "add") {
    enrollFingerprint(targetID);
    operation = "verify";
  } else if (operation == "delete") {
    deleteFingerprint(targetID);
    operation = "verify";
  } else if (operation == "update") {
    deleteFingerprint(targetID);
    delay(500);
    enrollFingerprint(targetID);
    sendUpdateMessage(targetID);
    operation = "verify";
  }
}
