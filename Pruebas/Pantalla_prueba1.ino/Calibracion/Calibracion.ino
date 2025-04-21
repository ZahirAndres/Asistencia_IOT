#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Pines del touch
#define TOUCH_CS 33
#define TOUCH_IRQ 36
#define TOUCH_MOSI 32
#define TOUCH_MISO 39
#define TOUCH_CLK 25

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(VSPI);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// --- Parámetros de calibración ---
struct CalPoint {
  int rawX, rawY;
  int scrX, scrY;
};
CalPoint cal[3];
bool calibrated = false;

// Coeficientes afines
float A, B, C, D, E, F;

// Dibuja una cruz en (x,y) en pantalla
void drawCross(int x, int y) {
  tft.drawLine(x - 12, y, x + 12, y, TFT_WHITE);
  tft.drawLine(x, y - 12, x, y + 12, TFT_WHITE);
}

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  touchSPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  touch.begin(touchSPI);
  touch.setRotation(3);

  tft.println("Calibracion 3 pts");
  delay(1000);
}

void loop() {
  if (!calibrated) {
    // 3 puntos: UL, UR, centro inferior
    static int step = 0;
    // Define pantallas para cada punto
    int sx[3] = { 20, tft.width() - 20, tft.width() / 2 };
    int sy[3] = { 20, 20, tft.height() - 20 };

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(10, 10);
    tft.printf("Paso %d/3: toca cruz", step + 1);
    drawCross(sx[step], sy[step]);

    // Espera toque
    if (touch.tirqTouched() && touch.touched()) {
      delay(50);
      TS_Point p = touch.getPoint();
      cal[step] = { p.x, p.y, sx[step], sy[step] };
      Serial.printf("P%d raw=(%4d,%4d) scr=(%3d,%3d)\n",
                    step + 1, p.x, p.y, sx[step], sy[step]);
      step++;
      delay(500);
      if (step >= 3) {
        // --- Calcular coeficientes afines ---
        // Montamos la matriz M de raw:
        // [ x0 y0 1 ]
        // [ x1 y1 1 ]
        // [ x2 y2 1 ]
        float x0 = cal[0].rawX, y0 = cal[0].rawY;
        float x1 = cal[1].rawX, y1 = cal[1].rawY;
        float x2 = cal[2].rawX, y2 = cal[2].rawY;
        // Determinante de M
        float det = x0 * (y1 - y2) - y0 * (x1 - x2) + (x1 * y2 - y1 * x2);
        // Inversa (adjunta / det)
        float inv[3][3];
        inv[0][0] = (y1 - y2) / det;
        inv[0][1] = -(y0 - y2) / det;
        inv[0][2] = (y0 - y1) / det;
        inv[1][0] = -(x1 - x2) / det;
        inv[1][1] = (x0 - x2) / det;
        inv[1][2] = -(x0 - x1) / det;
        inv[2][0] = (x1 * y2 - y1 * x2) / det;
        inv[2][1] = -(x0 * y2 - y0 * x2) / det;
        inv[2][2] = (x0 * y1 - y0 * x1) / det;
        // Vector de scrX = [ scr0, scr1, scr2 ] = [20, width-20, width/2]
        float s0x = cal[0].scrX, s1x = cal[1].scrX, s2x = cal[2].scrX;
        // Vector de scrY = [ scr0y, scr1y, scr2y ] = [20,20,height-20]
        float s0y = cal[0].scrY, s1y = cal[1].scrY, s2y = cal[2].scrY;
        // Calcula A,B,C  para X = A*rawX + B*rawY + C
        A = inv[0][0] * s0x + inv[0][1] * s1x + inv[0][2] * s2x;
        B = inv[1][0] * s0x + inv[1][1] * s1x + inv[1][2] * s2x;
        C = inv[2][0] * s0x + inv[2][1] * s1x + inv[2][2] * s2x;
        // Calcula D,E,F para Y = D*rawX + E*rawY + F
        D = inv[0][0] * s0y + inv[0][1] * s1y + inv[0][2] * s2y;
        E = inv[1][0] * s0y + inv[1][1] * s1y + inv[1][2] * s2y;
        F = inv[2][0] * s0y + inv[2][1] * s1y + inv[2][2] * s2y;
        Serial.println("\nCoeficientes afines:");
        Serial.printf("X = %.6f*x + %.6f*y + %.3f\n", A, B, C);
        Serial.printf("Y = %.6f*x + %.6f*y + %.3f\n", D, E, F);
        calibrated = true;
      }
    }
  } else {
    // --- Modo operación: transforma cada toque y dibuja punto ---
    if (touch.tirqTouched() && touch.touched()) {
      TS_Point p = touch.getPoint();
      int scrX = A * p.x + B * p.y + C;
      int scrY = D * p.x + E * p.y + F;
      // Limpia zona de lectura
      tft.fillRect(0, 30, tft.width(), 30, TFT_BLACK);
      tft.setCursor(10, 35);
      tft.printf("Scr=(%3d,%3d)", scrX, scrY);
      // Dibuja el punto calibrado
      tft.fillCircle(scrX, scrY, 5, TFT_RED);
      delay(150);
    }
  }
}
