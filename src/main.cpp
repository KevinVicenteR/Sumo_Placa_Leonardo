#include <Arduino.h>
#include "Motor.H"
#include "ControladorRobot.H"
#include "ConfiguracionHardware.H"
#include "Percepcion.H"
#include "EstrategiaCombate.H"
#include "ControlMovimiento.H"
#include "Parametros.H"
#include "Pines.H"

Motor motor;
Percepcion percepcion;
EstrategiaCombate estrategia;
ControlMovimiento controlMovimiento;
ControladorRobot robot(percepcion, estrategia, controlMovimiento, motor);
ConfiguracionHardware hardware;

#ifdef MODO_DIAGNOSTICO
// Motores apagados; imprime por USB lo que ve cada sensor para calibrar
void setup() {
  hardware.inicializarPines();
  motor.detener();
  Serial.begin(115200);
}

void loop() {
  Serial.print(F("pisoIzq=")); Serial.print(analogRead(S_PISO_IZQ));
  Serial.print(F(" pisoDer=")); Serial.print(analogRead(S_PISO_DER));
  Serial.print(F(" | latIzq=")); Serial.print(digitalRead(S_LAT_IZQ));
  Serial.print(F(" c45Izq=")); Serial.print(digitalRead(S_FRONT_IZQ));
  Serial.print(F(" frontal=")); Serial.print(digitalRead(S_FRONT_CEN));
  Serial.print(F(" c45Der=")); Serial.print(digitalRead(S_FRONT_DER));
  Serial.print(F(" latDer=")); Serial.println(digitalRead(S_LAT_DER));
  delay(100);
}
#else
void setup() {
  hardware.inicializarPines();
  motor.detener();

  // El robot arranca sobre el negro: se aprovecha la espera reglamentaria para calibrar
  percepcion.calibrarPiso();
  delay(TiempoInicioReglamentario);
}

void loop() {
  robot.actualizar();
}
#endif
