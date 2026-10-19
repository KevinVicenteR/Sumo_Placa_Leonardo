#include <Arduino.h>
#include "hardware/DriverMotores.h"
#include "configuracion/Pines.h"
#include "configuracion/Parametros.h"

void DriverMotores::controlarMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int velocidad) {
    velocidad = constrain(velocidad, -VelocidadMaxima, VelocidadMaxima);
    digitalWrite(in1, velocidad >= 0 ? HIGH : LOW);
    digitalWrite(in2, velocidad >= 0 ? LOW : HIGH);
    analogWrite(pwm, abs(velocidad));
}

void DriverMotores::mover(int velIzq, int velDer) {
    controlarMotor(MA1A, MA2A, PWMA, velIzq);
    controlarMotor(MA1B, MA2B, PWMB, velDer);
}

void DriverMotores::detener() { deshabilitar(); }

void DriverMotores::deshabilitar() {
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    digitalWrite(MA1A, LOW);
    digitalWrite(MA2A, LOW);
    digitalWrite(MA1B, LOW);
    digitalWrite(MA2B, LOW);
}
