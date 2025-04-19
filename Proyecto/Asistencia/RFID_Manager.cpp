#include "RFID_Manager.h"
#include <Arduino.h>
#include "FingerprintManager.h" // Para poder llamar a sendIDLogin desde este módulo
#include "BuzzerManager.h"

// Definimos los pines del RFID
#define RST_PIN 4    
#define SS_PIN  5    

MFRC522 rfid(SS_PIN, RST_PIN);
String lastRFID = "";

void leerRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidStr += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();
  lastRFID = uidStr;
  Serial.println("UID leída: " + lastRFID);
  beep();
  sendIDLogin(-1, lastRFID);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
