#include <Arduino.h>
#include "hardware/DriverMotores.h"
#include "Pines.h"
#include "Parametros.h"

// Un motor: el signo elige el sentido y el valor absoluto la potencia
void DriverMotores::controlarMotor(uint8_t dir, uint8_t pwm, int velocidad) {
    velocidad = constrain(velocidad, -VelocidadMaxima, VelocidadMaxima);
    digitalWrite(dir, velocidad >= 0 ? HIGH : LOW);
    analogWrite(pwm, abs(velocidad));
}

void DriverMotores::mover(int velIzq, int velDer) {
    controlarMotor(DIR_IZQ, PWM_IZQ, velIzq);
    controlarMotor(DIR_DER, PWM_DER, velDer);
}

void DriverMotores::detener() { deshabilitar(); }

void DriverMotores::deshabilitar() {
    // Primero la potencia a 0 y después los pines de sentido a LOW
    analogWrite(PWM_IZQ, 0);
    analogWrite(PWM_DER, 0);
    digitalWrite(DIR_IZQ, LOW);
    digitalWrite(DIR_DER, LOW);
}
