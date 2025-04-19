#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <base64.h> 
#include <SPI.h>
#include <MFRC522.h>  // Lector RFID

// Declaración de la función sendIDLogin
void sendIDLogin(int idHuella, String rfid);

// WiFi & MQTT
const char* ssid = "MrMexico2014";
const char* password = "Mr2014..";
const char* mqtt_server = "192.168.0.10";
const int mqtt_port = 1883;

const char* topic_add = "api/huella/add";
const char* topic_delete = "api/huella/delete";
const char* topic_update = "api/huella/update";

const char* topic_store = "api/huella/store";
const char* topic_login = "api/huella/login";
const char* topic_delete_result = "api/huella/delete/result";
const char* topic_update_result = "api/huella/update/result";

WiFiClient espClient;
PubSubClient client(espClient);

// Fingerprint sensor
HardwareSerial mySerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// RFID
#define RST_PIN 4    
#define SS_PIN  5    
MFRC522 rfid(SS_PIN, RST_PIN); 
String lastRFID = "";  

// Estado
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
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.print("Error al parsear JSON: ");
    Serial.println(error.c_str());
    return;
  }

  targetID = doc["idHuella"];
  idUsuarioDB = doc["idUsuario"];

  if (strcmp(topic, topic_add) == 0) operation = "add";
  else if (strcmp(topic, topic_delete) == 0) operation = "delete";
  else if (strcmp(topic, topic_update) == 0) operation = "update";

  Serial.printf("Operación: %s | ID Huella: %d | ID Usuario: %d\n", 
                operation.c_str(), targetID, idUsuarioDB);
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);
  SPI.begin();
  rfid.PCD_Init();

  connectToWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("Buscando sensor de huella...");
  if (finger.verifyPassword()) Serial.println("Sensor encontrado");
  else {
    Serial.println("Sensor no encontrado");
    while (1) delay(1);
  }
}

void loop() {
  if (!client.connected()) reconnectMQTT();
  client.loop();

  leerRFID();  // Actualiza siempre la UID

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
  sendIDLogin(-1, lastRFID);
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void enrollFingerprint(uint8_t id) {
  if (isFingerprintIDRegistered(id)) {
    Serial.println("El ID ya está registrado. No se puede sobrescribir.");
    return;
  }

  int p = -1;
  Serial.println("Coloca el dedo...");
  while (p != FINGERPRINT_OK) p = finger.getImage();
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return;
  Serial.println("Quita el dedo...");
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
    sendIDToDatabase(id);
  } else {
    Serial.println("Fallo al guardar huella");
  }
}

bool isFingerprintIDRegistered(uint8_t id) {
  return (finger.loadModel(id) == FINGERPRINT_OK);
}

void deleteFingerprint(uint8_t id) {
  int p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Huella eliminada");
    sendDeleteMessage(id);
  } else {
    Serial.println("Fallo al eliminar huella");
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
