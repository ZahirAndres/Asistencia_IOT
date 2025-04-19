#ifndef BUZZER_MANAGER_H
#define BUZZER_MANAGER_H

#include <Arduino.h>

// Define el pin del buzzer
#define BUZZER_PIN 15

// Inicializa el buzzer
void initBuzzer();

// Pitido corto
void beep(uint16_t frequency = 1000, uint16_t duration = 100);

// Reproduce una melodía de bienvenida o prueba
void playMelody();

// Reproduce una melodía suave de alerta
void playAlertMelody();

// Reproduce una melodía de proceso exitoso
void playSuccessMelody();

// Reproduce una melodía de fallo
void playErrorMelody();


#endif
