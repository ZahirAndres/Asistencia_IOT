#include "RgbLed.h"

RgbLed::RgbLed(int r, int g, int b) {
  pinR = r;
  pinG = g;
  pinB = b;
}

void RgbLed::begin() {
  if (pinR != LED_UNUSED) pinMode(pinR, OUTPUT);
  if (pinG != LED_UNUSED) pinMode(pinG, OUTPUT);
  if (pinB != LED_UNUSED) pinMode(pinB, OUTPUT);
  apagar();
}

void RgbLed::encenderColor(bool r, bool g, bool b) {
  if (pinR != LED_UNUSED) digitalWrite(pinR, r ? HIGH : LOW);
  if (pinG != LED_UNUSED) digitalWrite(pinG, g ? HIGH : LOW);
  if (pinB != LED_UNUSED) digitalWrite(pinB, b ? HIGH : LOW);
}

void RgbLed::encenderRojo() {
  encenderColor(true, false, false);
}

void RgbLed::encenderVerde() {
  encenderColor(false, true, false);
}

void RgbLed::encenderAzul() {
  encenderColor(false, false, true);
}

void RgbLed::encenderAmarillo() {
  encenderColor(true, true, false);
}

void RgbLed::encenderBlanco() {
  encenderColor(true, true, true);
}

void RgbLed::apagar() {
  encenderColor(false, false, false);
}
