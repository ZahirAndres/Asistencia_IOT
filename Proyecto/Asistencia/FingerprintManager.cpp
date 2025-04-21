#include "FingerprintManager.h"
#include "WiFi_MQTT_Manager.h"  // Para acceder a las variables y el cliente MQTT
#include "BuzzerManager.h"
#include "RFID_Manager.h"
#include "ClockManager.h"

#include <Arduino.h>

// Inicializamos el puerto serial y el objeto para el sensor de huella
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

extern PubSubClient client;
extern const char* topic_result;
// La variable lastRFID se definirá en otro módulo, pero aquí la declaramos como externa
extern String lastRFID;

bool isFingerprintIDRegistered(uint8_t id) {
  return (finger.loadModel(id) == FINGERPRINT_OK);
}

// Función para imprimir por Serial y publicar por MQTT
void logAndPublish(const String& message) {
  Serial.println(message);
  client.publish(topic_result, message.c_str());
}


void enrollFingerprint(uint8_t id) {
  if (isFingerprintIDRegistered(id)) {
    logAndPublish("El ID ya está registrado. No se puede sobrescribir.");
    playAlertMelody();
    return;
  }

  // Leer RFID antes de capturar la huella
  logAndPublish("🔷 Por favor, acerque la tarjeta RFID...");
  lastRFID = "";  // Limpia la última lectura
  unsigned long startTime = millis();
  const unsigned long timeout = 10000; // 10 segundos para leer RFID

  while (lastRFID == "" && (millis() - startTime < timeout)) {
    leerRFID();
    delay(200);  // Pausa para evitar saturación
  }

  if (lastRFID == "") {
    logAndPublish("Tiempo agotado. No se detectó RFID.");
    playAlertMelody();
    return;
  }

  logAndPublish("RFID detectado: " + lastRFID);
  playSuccessMelody();
  delay(1000);

  int p = -1;
  logAndPublish("Coloca el dedo...");
  while (p != FINGERPRINT_OK) p = finger.getImage();

  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return;

  logAndPublish("Quita el dedo...");
  beep();
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  logAndPublish("Coloca el mismo dedo otra vez...");
  while (finger.getImage() != FINGERPRINT_OK);

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return;

  p = finger.createModel();
  if (p != FINGERPRINT_OK) return;

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    logAndPublish("Huella registrada correctamente");
    logAndPublish("");
    playSuccessMelody();
    sendIDToDatabase(id);
  } else {
    logAndPublish("Fallo al guardar huella");
    playAlertMelody();
  }
}

void deleteFingerprint(uint8_t id) {
  int p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Huella eliminada");
    playSuccessMelody();
    sendDeleteMessage(id);
  } else {
    Serial.println("Fallo al eliminar huella");
    playErrorMelody();
  }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return p;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Huella encontrada! ID: ");
    beep();
    Serial.println(finger.fingerID);
    sendIDLogin(finger.fingerID, "");
  }
  return p;
}

void sendIDToDatabase(uint8_t idHuella) {
  StaticJsonDocument<256> doc;
  doc["idHuella"] = idHuella;
  doc["idUsuario"] = idUsuarioDB;
  doc["tipo_usuario"] = "Alumno";
  doc["rfid"] = lastRFID;
  
  String output;
  serializeJson(doc, output);
  client.publish(topic_store, output.c_str());
  Serial.println("Datos enviados:");
  Serial.println(output);
}

void sendDeleteMessage(uint8_t id) {
  StaticJsonDocument<128> doc;
  doc["id"] = id;
  doc["mensaje"] = "Eliminación correcta";
  
  String output;
  serializeJson(doc, output);
  client.publish(topic_delete_result, output.c_str());
}

void sendIDLogin(int idHuella, String rfid) {
  StaticJsonDocument<256> doc;

  if (idHuella != -1) {
    doc["huella"] = idHuella;
  }

  if (rfid != "") {
    doc["rfid"] = rfid;
  }

  // Obtener hora actual del sistema
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  doc["hora"] = buffer;

  String output;
  serializeJson(doc, output);
  client.publish(topic_asis_huella, output.c_str());

  Serial.println("Datos de login enviados:");
  Serial.println(output);
}


void sendUpdateMessage(uint8_t id) {
  StaticJsonDocument<128> doc;
  doc["id"] = id;
  doc["mensaje"] = "Huella actualizada";
  
  String output;
  serializeJson(doc, output);
  client.publish(topic_update_result, output.c_str());
}
