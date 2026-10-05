#include <Arduino.h>
#include "ConfiguracionHardware.H"
#include "Pines.H"
#include "Parametros.H"

void ConfiguracionHardware::inicializarPines() const {
    digitalWrite(PWMA, LOW);
    digitalWrite(PWMB, LOW);
    pinMode(PWMA, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(S_FRONT_IZQ, INPUT);
    pinMode(S_FRONT_CEN, INPUT);
    pinMode(S_FRONT_DER, INPUT);
    pinMode(S_LAT_IZQ, INPUT);
    pinMode(S_LAT_DER, INPUT);
    if (PIN_MODULO_ARRANQUE >= 0) {
        pinMode((uint8_t)PIN_MODULO_ARRANQUE, INPUT);
    }
    pinMode(DIP_1, INPUT_PULLUP);
    pinMode(DIP_2, INPUT_PULLUP);
    pinMode(DIP_3, INPUT_PULLUP);

    digitalWrite(MA1A, LOW);
    digitalWrite(MA2A, LOW);
    digitalWrite(MA1B, LOW);
    digitalWrite(MA2B, LOW);
    pinMode(MA1A, OUTPUT);
    pinMode(MA2A, OUTPUT);
    pinMode(MA1B, OUTPUT);
    pinMode(MA2B, OUTPUT);
}

int ConfiguracionHardware::leerInterruptores() const {
    const uint8_t pines[3] = {DIP_1, DIP_2, DIP_3};
    int bits = 0;
    for (int i = 0; i < 3; i++) {
        const bool alto = digitalRead(pines[i]) == HIGH;
        if (alto != DipActivoBajo) bits |= 1 << i;
    }
    return bits;
}
