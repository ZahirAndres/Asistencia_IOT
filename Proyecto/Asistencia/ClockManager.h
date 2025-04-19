#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#include <TM1637Display.h>

#define CLK_PIN 32  // Puedes cambiar estos si lo necesitas
#define DIO_PIN 33

void initClock();
void updateClock();

#endif
