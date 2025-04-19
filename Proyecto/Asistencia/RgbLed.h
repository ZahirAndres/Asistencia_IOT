#ifndef RGBLED_H
#define RGBLED_H

#define LED_UNUSED -1

#include <Arduino.h>

class RgbLed {
  private:
    int pinR, pinG, pinB;

  public:
    RgbLed(int r, int g, int b);
    void begin();
    void encenderColor(bool r, bool g, bool b);
    void encenderRojo();
    void encenderVerde();
    void encenderAzul();
    void encenderAmarillo();
    void encenderBlanco();
    void apagar();
};

#endif
