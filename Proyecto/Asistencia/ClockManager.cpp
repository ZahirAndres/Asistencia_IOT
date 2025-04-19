#include "ClockManager.h"
#include "WiFi_MQTT_Manager.h"
#include <Arduino.h>
#include <TM1637Display.h>
#include <time.h>

// Crear una instancia del display TM1637
TM1637Display display(CLK_PIN, DIO_PIN);

void initClock() {
  // Inicializar el display y configurar el nivel de brillo (0x0f es el brillo máximo)
  display.setBrightness(0x0f);

  // Ya no reconectamos WiFi ni MQTT aquí.
  // Se asume que ya están conectados al llegar a este punto.

  // Configurar la hora con NTP usando la red Wi-Fi
  configTime(-21600, 0, "pool.ntp.org", "time.nist.gov");

  // Esperar a que el tiempo esté disponible
  time_t now = time(nullptr);
  while (now < 100000) {
    delay(500);
    now = time(nullptr);
  }
  Serial.println("Hora actualizada vía NTP");
}

void updateClock() {
  // Obtener la hora actual
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  // Extraer las horas y minutos
  int displayTime = timeinfo.tm_hour * 100 + timeinfo.tm_min;

  // Mostrar la hora en el display con los dos puntos intermitentes
  display.showNumberDecEx(displayTime, 0b01000000, true);
}
