#include <Arduino.h>
#include "Motor.H"
#include "Pines.H"
#include "Parametros.H"

namespace {
void controlarMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int velocidad) {
    velocidad = constrain(velocidad, -VelocidadMaxima, VelocidadMaxima);
    digitalWrite(in1, velocidad >= 0 ? HIGH : LOW);
    digitalWrite(in2, velocidad >= 0 ? LOW : HIGH);
    analogWrite(pwm, abs(velocidad));
}
}

void Motor::mover(int velIzq, int velDer) {
    controlarMotor(MA1A, MA2A, PWMA, InvertirMotorIzq ? -velIzq : velIzq);
    controlarMotor(MA1B, MA2B, PWMB, InvertirMotorDer ? -velDer : velDer);
}

void Motor::detener() { deshabilitar(); }

void Motor::deshabilitar() {
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    digitalWrite(MA1A, LOW);
    digitalWrite(MA2A, LOW);
    digitalWrite(MA1B, LOW);
    digitalWrite(MA2B, LOW);
}
