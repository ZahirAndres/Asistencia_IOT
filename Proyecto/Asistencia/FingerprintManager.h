#ifndef FINGERPRINT_MANAGER_H
#define FINGERPRINT_MANAGER_H

#include <Adafruit_Fingerprint.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <PubSubClient.h>

// Declaramos las variables que se usarán en este módulo
extern HardwareSerial mySerial;
extern Adafruit_Fingerprint finger;

// Declaramos que la variable lastRFID se usará en otro módulo (RFID)
extern String lastRFID;

// Funciones relacionadas al sensor de huella
bool isFingerprintIDRegistered(uint8_t id);
void enrollFingerprint(uint8_t id);
void deleteFingerprint(uint8_t id);
uint8_t getFingerprintID();

// Funciones para enviar mensajes vía MQTT
void sendIDToDatabase(uint8_t idHuella);
void sendIDLogin(int idHuella, String rfid);
void sendDeleteMessage(uint8_t id);
void sendUpdateMessage(uint8_t id);

#endif
