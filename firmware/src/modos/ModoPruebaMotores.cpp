// Solo se compila en el entorno "prueba_motores" (usa el monitor serie)
#if defined(MODO_PRUEBA_MOTORES)
#include <Arduino.h>
#include "modos/ModoPruebaMotores.h"

void ModoPruebaMotores::iniciar() {
    placa.inicializarPines();
    motores.deshabilitar();
    Serial.begin(115200);
    delay(3000);
}

void ModoPruebaMotores::paso(const __FlashStringHelper* texto, int izq, int der) {
    Serial.println(texto);
    motores.mover(izq, der);
    delay(1500);
    motores.detener();
    delay(1000);
}

void ModoPruebaMotores::actualizar() {
    // Cada rueda sola, a tres potencias
    paso(F("IZQUIERDA sola: PWM D10=100"), 100, 0);
    paso(F("IZQUIERDA sola: PWM D10=160"), 160, 0);
    paso(F("IZQUIERDA sola: PWM D10=220"), 220, 0);
    paso(F("DERECHA sola: PWM D11=100"), 0, 100);
    paso(F("DERECHA sola: PWM D11=160"), 0, 160);
    paso(F("DERECHA sola: PWM D11=220"), 0, 220);
    // Cada rueda en los dos sentidos
    paso(F("IZQUIERDA adelante: PWM D10=180, D8/D12"), 180, 0);
    paso(F("IZQUIERDA atras: PWM D10=180, D8/D12"), -180, 0);
    paso(F("DERECHA adelante: PWM D11=180, D9/D13"), 0, 180);
    paso(F("DERECHA atras: PWM D11=180, D9/D13"), 0, -180);
    // Las dos juntas
    paso(F("Las dos hacia DELANTE"), 120, 120);
    paso(F("Las dos hacia ATRAS"), -120, -120);
    paso(F("Giro en el sitio a la DERECHA (horario visto desde arriba)"), 120, -120);
    paso(F("Parada: las ruedas deben quedarse QUIETAS"), 0, 0);
    Serial.println(F("--- repite ---"));
}
#endif
