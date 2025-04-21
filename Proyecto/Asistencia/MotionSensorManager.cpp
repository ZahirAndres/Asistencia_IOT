#include "MotionSensorManager.h"
#include <Arduino.h>
#include "WiFi_MQTT_Manager.h" // Se accede al objeto MQTT (client) declarado en el módulo WiFi/MQTT
#include "BuzzerManager.h"

// Variable interna para almacenar el estado previo del sensor
bool motionDetected = false;

void initMotionSensor() {
  // Configura el pin del sensor como entrada
  pinMode(MOTION_SENSOR_PIN, INPUT);
}

void checkMotionSensor() {
  int sensorValue = digitalRead(MOTION_SENSOR_PIN);
  
  // Si se detecta movimiento (valor HIGH) y anteriormente no se había detectado
  if (sensorValue == HIGH && !motionDetected) {
    motionDetected = true;
    Serial.println("Movimiento detectado: persona detectada");
    playMelody();
    // Publica el mensaje en el tópico correspondiente
    client.publish("api/sensor/movimiento", "Bienvenido !!!");
  }
  // Si no hay detección, reseteamos la bandera para detectar un futuro movimiento
  else if (sensorValue == LOW) {
    motionDetected = false;
  }
}
