#include <Arduino.h>
#include "ConfiguracionHardware.H"
#include "Pines.H"

void ConfiguracionHardware::inicializarPines() const {
    // Deshabilitar los puentes antes de configurar sus entradas de dirección.
    digitalWrite(PWMA, LOW);
    digitalWrite(PWMB, LOW);
    pinMode(PWMA, OUTPUT);
    pinMode(PWMB, OUTPUT);
    digitalWrite(LED_IZQ, LOW);
    digitalWrite(LED_DER, LOW);
    pinMode(LED_IZQ, OUTPUT);
    pinMode(LED_DER, OUTPUT);
    pinMode(S_FRONT_IZQ, INPUT);
    pinMode(S_FRONT_CEN, INPUT);
    pinMode(S_FRONT_DER, INPUT);
    pinMode(S_LAT_IZQ, INPUT);
    pinMode(S_LAT_DER, INPUT);

    digitalWrite(DIR_IZQ, LOW);
    digitalWrite(DIR_DER, LOW);
    pinMode(DIR_IZQ, OUTPUT);
    pinMode(DIR_DER, OUTPUT);
}
