#include <Arduino.h>
#include "hardware/DriverMotores.h"
#include "Pines.h"
#include "Parametros.h"

// Un motor: el signo elige el sentido y el valor absoluto la potencia
void DriverMotores::controlarMotor(uint8_t in1, uint8_t in2, uint8_t pwm, int velocidad) {
    velocidad = constrain(velocidad, -VelocidadMaxima, VelocidadMaxima);
    digitalWrite(in1, velocidad >= 0 ? HIGH : LOW);
    digitalWrite(in2, velocidad >= 0 ? LOW : HIGH);
    analogWrite(pwm, abs(velocidad));
}

void DriverMotores::mover(int velIzq, int velDer) {
    // Si un motor está cableado al revés, se invierte su orden (ver InvertirMotor*)
    controlarMotor(MA1A, MA2A, PWMA, InvertirMotorIzq ? -velIzq : velIzq);
    controlarMotor(MA1B, MA2B, PWMB, InvertirMotorDer ? -velDer : velDer);
}

void DriverMotores::detener() { deshabilitar(); }

void DriverMotores::deshabilitar() {
    // Primero la potencia a 0 y después los pines de sentido a LOW
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
    digitalWrite(MA1A, LOW);
    digitalWrite(MA2A, LOW);
    digitalWrite(MA1B, LOW);
    digitalWrite(MA2B, LOW);
}
