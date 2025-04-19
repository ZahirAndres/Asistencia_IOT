#ifndef RFID_MANAGER_H
#define RFID_MANAGER_H

#include <SPI.h>
#include <MFRC522.h>

// Declaramos que el objeto rfid y la variable lastRFID serán globales
extern MFRC522 rfid;
extern String lastRFID;

// Función para leer el RFID
void leerRFID();

#endif
