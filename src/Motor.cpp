#include <Arduino.h>
#include "Motor.H"
#include "Pines.H"
#include "Parametros.H"

namespace {
void controlarPuente(uint8_t pinIn1, uint8_t pinIn2, uint8_t pinPwm, int velocidad) {
	velocidad = constrain(velocidad, -VelocidadMaxima, VelocidadMaxima);
	digitalWrite(pinIn1, velocidad >= 0 ? HIGH : LOW);
	digitalWrite(pinIn2, velocidad >= 0 ? LOW : HIGH);
	analogWrite(pinPwm, abs(velocidad));
}
}

void Motor::mover(int velIzq, int velDer) {
	controlarPuente(MA1A, MA2A, PWMA, velIzq); // Motor izquierdo (A)
	controlarPuente(MA1B, MA2B, PWMB, velDer); // Motor derecho (B)
}

void Motor::detener() {
	mover(0, 0);
}
