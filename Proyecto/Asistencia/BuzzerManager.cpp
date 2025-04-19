#include "BuzzerManager.h"

void initBuzzer() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void beep(uint16_t frequency, uint16_t duration) {
  tone(BUZZER_PIN, frequency, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

// Notas musicales
#define NOTE_C4  262
#define NOTE_C4S 277
#define NOTE_D4  294
#define NOTE_D4S 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_F4S 370
#define NOTE_G4  392
#define NOTE_G4S 415
#define NOTE_A4  440
#define NOTE_A4S 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_C5S 554
#define NOTE_D5  587
#define NOTE_D5S 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_F5S 740
#define NOTE_G5  784
#define NOTE_G5S 830
#define NOTE_A5  880
#define NOTE_A5S 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_C6S 1109
#define NOTE_D6  1175
#define NOTE_D6S 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_F6S 1480
#define NOTE_G6  1568
#define NOTE_G6S 1661
#define NOTE_A6  1760
#define NOTE_A6S 1865
#define NOTE_B6  1976

#define NOTE_REST 0

// Melodía básica (bienvenida o prueba)
void playMelody() {
  int melody[] = {
    NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4,
    NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5
  };

  int durations[] = {
    300, 300, 300, 300,
    300, 300, 300, 500
  };

  for (int i = 0; i < 8; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] * 1.3);
    noTone(BUZZER_PIN);
  }
}

// 🎵 Alerta suave cuando el sensor no está disponible
void playAlertMelody() {
  int melody[] = {
    NOTE_E4, NOTE_REST, NOTE_E4, NOTE_REST, NOTE_D4
  };

  int durations[] = {
    150, 100, 150, 100, 300
  };

  for (int i = 0; i < 5; i++) {
    if (melody[i] != NOTE_REST) {
      tone(BUZZER_PIN, melody[i], durations[i]);
    }
    delay(durations[i]);
    noTone(BUZZER_PIN);
  }
}


// 🎉 Melodía de éxito (alegre y breve)
void playSuccessMelody() {
  int melody[] = {
    NOTE_E4, NOTE_G4, NOTE_C5, NOTE_E5
  };

  int durations[] = {
    150, 150, 150, 300
  };

  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] * 1.3);
    noTone(BUZZER_PIN);
  }
}


// 🔴 Melodía de error (grave y corta)
void playErrorMelody() {
  int melody[] = {
    NOTE_G4, NOTE_E4, NOTE_C4
  };

  int durations[] = {
    200, 200, 400
  };

  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, melody[i], durations[i]);
    delay(durations[i] * 1.3);
    noTone(BUZZER_PIN);
  }
}
