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

namespace {
// Se actualiza desde el programa, nunca desde una interrupción: si el código
// se bloquea, los LED dejan de cambiar aunque millis siga avanzando.
void indicarActividad() {
  static unsigned long ultimoCambio = 0;
  static bool encendidos = false;
  const unsigned long ahora = millis();
  if (ahora - ultimoCambio >= 250) {
    ultimoCambio = ahora;
    encendidos = !encendidos;
    digitalWrite(LED_IZQ, encendidos ? HIGH : LOW);
    digitalWrite(LED_DER, encendidos ? HIGH : LOW);
  }
}

void esperarConIndicadores(unsigned long duracion) {
  const unsigned long inicio = millis();
  while (millis() - inicio < duracion) {
    indicarActividad();
    delay(1);
  }
}
}

#if defined(MODO_PRUEBA_MOTORES)
// Prueba de motores con el robot levantado (ruedas en el aire): repite una
// secuencia de movimientos y la anuncia por USB para comprobar que cada rueda
// gira hacia donde dice
namespace {
void paso(const __FlashStringHelper* texto, int izq, int der) {
  Serial.println(texto);
  motor.mover(izq, der);
  esperarConIndicadores(1500);
  motor.detener();
  esperarConIndicadores(1000);
}
}

void setup() {
  hardware.inicializarPines();
  motor.deshabilitar();
  Serial.begin(115200);
  esperarConIndicadores(3000);
}

void loop() {
  paso(F("IZQUIERDA adelante: PWM D3=180, DIR D12=HIGH"), 180, 0);
  paso(F("IZQUIERDA atras: PWM D3=180, DIR D12=LOW"), -180, 0);
  paso(F("DERECHA adelante: PWM D11=180, DIR D13=HIGH"), 0, 180);
  paso(F("DERECHA atras: PWM D11=180, DIR D13=LOW"), 0, -180);
  paso(F("Las dos hacia DELANTE"), 120, 120);
  paso(F("Las dos hacia ATRAS"), -120, -120);
  paso(F("Giro en el sitio a la DERECHA (horario visto desde arriba)"), 120, -120);
  paso(F("Parada: las ruedas deben quedarse QUIETAS"), 0, 0);
  Serial.println(F("--- repite ---"));
}
#elif defined(MODO_DIAGNOSTICO)
// Motores apagados; imprime por USB lo que ve cada sensor para calibrar
void setup() {
  hardware.inicializarPines();
  motor.deshabilitar();
  Serial.begin(115200);
}

void loop() {
  const int izq = analogRead(S_PISO_IZQ);
  const int der = analogRead(S_PISO_DER);
  indicarActividad();
  Serial.print(F("pisoIzq=")); Serial.print(izq);
  Serial.print(F(" pisoDer=")); Serial.print(der);
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
  // Durante toda la espera y calibración, PWM=0 y entradas de dirección LOW.
  motor.deshabilitar();

  esperarConIndicadores(TiempoInicioReglamentario);
  // Ambos LED fijos durante la medición, sin alimentar los motores.
  digitalWrite(LED_IZQ, HIGH);
  digitalWrite(LED_DER, HIGH);
  // Se calibra al final de la espera: para entonces el robot ya está colocado
  // sobre el negro, aunque se haya encendido en la mano
  percepcion.calibrarPiso();
  percepcion.calibrarSensoresEnemigo();
  digitalWrite(LED_IZQ, LOW);
  digitalWrite(LED_DER, LOW);
}

void loop() {
  robot.actualizar();
  indicarActividad();
}
#endif
