#ifndef MOTION_SENSOR_MANAGER_H
#define MOTION_SENSOR_MANAGER_H

#include <Arduino.h>

// Define el pin al que está conectado el sensor de movimiento
#define MOTION_SENSOR_PIN 14

// Declaración de funciones para inicializar y checar el sensor de movimiento
void initMotionSensor();
void checkMotionSensor();

#endif
