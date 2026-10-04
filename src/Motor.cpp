#include <Arduino.h>
#include "Motor.H"
#include "Pines.H"
#include "Parametros.H"

namespace {
void controlarMotor(uint8_t direccion, uint8_t pwm, int velocidad) {
    velocidad = constrain(velocidad, -VelocidadMaxima, VelocidadMaxima);
    // PWM=0 equivale a StopMotors de XMotion; nunca PWM máximo al parar.
    // Mantener PWM entre ciclos como hace la biblioteca oficial de XMotion.
    digitalWrite(direccion, velocidad > 0 ? HIGH : LOW);
    analogWrite(pwm, abs(velocidad));
}
}

void Motor::mover(int velIzq, int velDer) {
    controlarMotor(DIR_IZQ, PWMA, velIzq);
    controlarMotor(DIR_DER, PWMB, velDer);
}

void Motor::detener() { deshabilitar(); }

void Motor::deshabilitar() {
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    digitalWrite(DIR_IZQ, LOW);
    digitalWrite(DIR_DER, LOW);
}
