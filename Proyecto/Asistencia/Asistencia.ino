#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <SPI.h>
#include <MFRC522.h>

// Módulos personalizados
#include "WiFi_MQTT_Manager.h"
#include "FingerprintManager.h"
#include "RFID_Manager.h"
#include "MotionSensorManager.h"
#include "BuzzerManager.h"
#include "ClockManager.h"
#include "RgbLed.h"

// LEDs
RgbLed led1(4, LED_UNUSED, LED_UNUSED);
RgbLed led2(25, 26, LED_UNUSED);
RgbLed led3(13, 21, 27);

// Banderas de soporte de color
bool led1SoportaRGB = false;
bool led2SoportaRGB = true;
bool led3SoportaRGB = true;

// Variables reconexión sensor huella
bool sensorHuellaConectado = false;
unsigned long ultimoIntentoHuella = 0;
const unsigned long intervaloReintento = 3000;
unsigned long ultimoBeep = 0;
const unsigned long intervaloBeep = 800;

// Funciones auxiliares de LEDs
void mostrarErrorSensorHuella() {
  if (led1SoportaRGB) led1.encenderAmarillo(); else led1.encenderRojo();
  if (led2SoportaRGB) led2.encenderAmarillo(); else led2.encenderRojo();
  led3.encenderRojo();
}

void mostrarTodoCorrecto() {
  led1.encenderVerde();
  led2.encenderVerde();
  led3.encenderVerde();
}

void setup() {
  Serial.begin(115200);
  led1.begin();
  led2.begin();
  led3.begin();

  // Serial para sensor huella
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);

  // Inicializar RFID
  SPI.begin();
  rfid.PCD_Init();

  // Inicializar otros módulos
  initBuzzer();
  initMotionSensor();

  // Conexión WiFi
  connectToWiFi();

  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  reconnectMQTT();

  // Validar conexión
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexión WiFi exitosa.");

    // Mostrar LEDs blancos
    led1.encenderBlanco();
    led2.encenderBlanco();
    led3.encenderBlanco();
    delay(1500);
    led1.apagar();
    led2.apagar();
    led3.apagar();

    initClock();

    // Validar sensor huella
    if (finger.verifyPassword()) {
      Serial.println("Sensor de huella detectado.");
          playSuccessMelody();

      sensorHuellaConectado = true;
    } else {
      Serial.println("Sensor de huella NO detectado en setup.");
      playErrorMelody();
      sensorHuellaConectado = false;
    }

  } else {
    Serial.println("Error: No conectado a WiFi.");
    playErrorMelody();
  }
}

void loop() {
  unsigned long ahora = millis();

  // Verificar WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado. Reintentando...");
    beep();
    connectToWiFi();
    led1.encenderRojo();

  }

  // Verificar reconexión sensor huella
  bool estadoActual = finger.verifyPassword();

  if (!estadoActual && sensorHuellaConectado) {
    Serial.println("⚠️ Sensor de huella desconectado.");
    led3.encenderRojo();
    sensorHuellaConectado = false;
    ultimoIntentoHuella = ahora;
  }

  if (!sensorHuellaConectado) {
    if (ahora - ultimoIntentoHuella >= intervaloReintento) {
      Serial.println("🔄 Intentando reconectar el sensor de huella...");
      if (finger.verifyPassword()) {
        Serial.println("✅ Sensor de huella reconectado.");
        sensorHuellaConectado = true;
        mostrarTodoCorrecto();
      } else {
        Serial.println("❌ Sensor de huella NO encontrado.");
        playAlertMelody(); // 🎵 Sonido suave para indicar fallo del sensor
        mostrarErrorSensorHuella();
      }
      ultimoIntentoHuella = ahora;
    }

    if (ahora - ultimoBeep >= intervaloBeep) {
      beep();
      ultimoBeep = ahora;
    }

    return; // Salir del loop mientras no haya sensor de huella
  }

  // Mostrar LEDs verdes si todo está bien
  if (sensorHuellaConectado && WiFi.status() == WL_CONNECTED && client.connected()) {
    mostrarTodoCorrecto();
  }

  // Reconectar MQTT si es necesario
  if (!client.connected()) {
    Serial.println("⚠️ MQTT desconectado. Reintentando...");
    beep();
    reconnectMQTT();
  }

  client.loop(); // Mantener la conexión MQTT activa

  // Funcionalidad principal
  leerRFID();
  checkMotionSensor();
  updateClock();

  // Operaciones de huella
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
