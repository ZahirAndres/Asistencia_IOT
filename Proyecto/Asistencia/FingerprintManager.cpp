#include "FingerprintManager.h"
#include "WiFi_MQTT_Manager.h"  // Para acceder a las variables y el cliente MQTT
#include "BuzzerManager.h"

#include <Arduino.h>

// Inicializamos el puerto serial y el objeto para el sensor de huella
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// La variable lastRFID se definirá en otro módulo, pero aquí la declaramos como externa
extern String lastRFID;

bool isFingerprintIDRegistered(uint8_t id) {
  return (finger.loadModel(id) == FINGERPRINT_OK);
}

void enrollFingerprint(uint8_t id) {
  if (isFingerprintIDRegistered(id)) {
    Serial.println("El ID ya está registrado. No se puede sobrescribir.");
    playAlertMelody();
    return;
  }
  
  int p = -1;
  Serial.println("Coloca el dedo...");
  while (p != FINGERPRINT_OK) p = finger.getImage();
  
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return;
  
  Serial.println("Quita el dedo...");
  beep();
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);
  
  Serial.println("Coloca el mismo dedo otra vez...");
  while (finger.getImage() != FINGERPRINT_OK);
  
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return;
  
  p = finger.createModel();
  if (p != FINGERPRINT_OK) return;
  
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Huella registrada correctamente");
    playSuccessMelody();
    sendIDToDatabase(id);
  } else {
    Serial.println("Fallo al guardar huella");
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
  doc["nombre"] = "Juan";
  doc["apellido"] = "Perez";
  doc["tipo_usuario"] = "Empleado";
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
  doc["status"] = "deleted";
  
  String output;
  serializeJson(doc, output);
  client.publish(topic_delete_result, output.c_str());
}

void sendIDLogin(int idHuella, String rfid) {
  StaticJsonDocument<128> doc;
  
  if (idHuella != -1) {
    doc["huella"] = idHuella;
  }
  
  if (rfid != "") {
    doc["rfid"] = rfid;
  }
  
  String output;
  serializeJson(doc, output);
  client.publish(topic_login, output.c_str());
}

void sendUpdateMessage(uint8_t id) {
  StaticJsonDocument<128> doc;
  doc["id"] = id;
  doc["status"] = "updated";
  
  String output;
  serializeJson(doc, output);
  client.publish(topic_update_result, output.c_str());
}
