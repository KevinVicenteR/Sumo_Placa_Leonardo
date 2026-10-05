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

#if defined(MODO_PRUEBA_MOTORES)
namespace {
void paso(const __FlashStringHelper* texto, int izq, int der) {
  Serial.println(texto);
  motor.mover(izq, der);
  delay(1500);
  motor.detener();
  delay(1000);
}
}

void setup() {
  hardware.inicializarPines();
  motor.deshabilitar();
  Serial.begin(115200);
  delay(3000);
}

void loop() {
  paso(F("IZQUIERDA sola: PWM D10=100"), 100, 0);
  paso(F("IZQUIERDA sola: PWM D10=160"), 160, 0);
  paso(F("IZQUIERDA sola: PWM D10=220"), 220, 0);
  paso(F("DERECHA sola: PWM D11=100"), 0, 100);
  paso(F("DERECHA sola: PWM D11=160"), 0, 160);
  paso(F("DERECHA sola: PWM D11=220"), 0, 220);
  paso(F("IZQUIERDA adelante: PWM D10=180, D9=HIGH, D13=LOW"), 180, 0);
  paso(F("IZQUIERDA atras: PWM D10=180, D9=LOW, D13=HIGH"), -180, 0);
  paso(F("DERECHA adelante: PWM D11=180, D8=HIGH, D12=LOW"), 0, 180);
  paso(F("DERECHA atras: PWM D11=180, D8=LOW, D12=HIGH"), 0, -180);
  paso(F("Las dos hacia DELANTE"), 120, 120);
  paso(F("Las dos hacia ATRAS"), -120, -120);
  paso(F("Giro en el sitio a la DERECHA (horario visto desde arriba)"), 120, -120);
  paso(F("Parada: las ruedas deben quedarse QUIETAS"), 0, 0);
  Serial.println(F("--- repite ---"));
}
#elif defined(MODO_DIAGNOSTICO)
namespace {
void calibrarDiagnostico() {
  motor.deshabilitar();
  percepcion.calibrarPiso();
  percepcion.calibrarSensoresEnemigo();
  Serial.print(F("CALIBRADO negroIzq=")); Serial.print(percepcion.negroIzquierdo());
  Serial.print(F(" margenIzq=")); Serial.print(percepcion.margenIzquierdo());
  Serial.print(F(" negroDer=")); Serial.print(percepcion.negroDerecho());
  Serial.print(F(" margenDer=")); Serial.println(percepcion.margenDerecho());
}
}
void setup() {
  hardware.inicializarPines();
  motor.deshabilitar();
  Serial.begin(115200);
  delay(TiempoInicioReglamentario);
  calibrarDiagnostico();
}

void loop() {
  if (Serial.available() && (Serial.read() == 'c')) calibrarDiagnostico();
  const LecturasSensores l = percepcion.leer();
  Serial.print(F("pisoIzq=")); Serial.print(analogRead(S_PISO_IZQ));
  Serial.print(F(" pisoDer=")); Serial.print(analogRead(S_PISO_DER));
  Serial.print(F(" | lineaIzq=")); Serial.print(l.lineaIzq);
  Serial.print(F(" lineaDer=")); Serial.print(l.lineaDer);
  Serial.print(F(" | RAW="));
  Serial.print(digitalRead(S_LAT_IZQ)); Serial.print(',');
  Serial.print(digitalRead(S_FRONT_IZQ)); Serial.print(',');
  Serial.print(digitalRead(S_FRONT_CEN)); Serial.print(',');
  Serial.print(digitalRead(S_FRONT_DER)); Serial.print(',');
  Serial.print(digitalRead(S_LAT_DER));
  Serial.print(F(" | DIP="));
  Serial.print(hardware.leerInterruptores() & 1 ? '1' : '0');
  Serial.print(hardware.leerInterruptores() & 2 ? '1' : '0');
  Serial.print(hardware.leerInterruptores() & 4 ? '1' : '0');
  Serial.print(F(" | VE="));
  Serial.print(l.latIzq); Serial.print(',');
  Serial.print(l.c45Izq); Serial.print(',');
  Serial.print(l.frontal); Serial.print(',');
  Serial.print(l.c45Der); Serial.print(',');
  Serial.println(l.latDer);
  delay(100);
}
#else
#include "ModuloArranque.H"

ModuloArranque moduloArranque(FiltroModuloArranqueMs);
bool enCombate = false;

void empezarCombate() {
  motor.deshabilitar();
  percepcion.calibrarPiso();
  controlMovimiento.reiniciar();
  const int dip = hardware.leerInterruptores();
  controlMovimiento.iniciarRutina(rutinaSegunInterruptores(dip), (dip & 4) ? -1 : 1);
  enCombate = true;
#if defined(MONITOREO_COMBATE)
  Serial.print(F("DIP=")); Serial.print(dip); Serial.print(' ');
  Serial.print(F("INICIO negroIzq=")); Serial.print(percepcion.negroIzquierdo());
  Serial.print(F(" margenIzq=")); Serial.print(percepcion.margenIzquierdo());
  Serial.print(F(" negroDer=")); Serial.print(percepcion.negroDerecho());
  Serial.print(F(" margenDer=")); Serial.println(percepcion.margenDerecho());
#endif
}

void setup() {
  hardware.inicializarPines();
  motor.deshabilitar();
#if defined(MONITOREO_COMBATE)
  Serial.begin(115200);
#endif
  if (PIN_MODULO_ARRANQUE < 0) {
    empezarCombate();
  }
}

void loop() {
  if (PIN_MODULO_ARRANQUE >= 0) {
    const bool alto = digitalRead((uint8_t)PIN_MODULO_ARRANQUE) == HIGH;
    if (!moduloArranque.enMarcha(alto == ModuloArranqueActivoAlto, millis())) {
      motor.deshabilitar();
      enCombate = false;
      return;
    }
    if (!enCombate) {
      empezarCombate();
    }
  }
  robot.actualizar();
#if defined(MONITOREO_COMBATE)
  static unsigned long ultimaTraza = 0;
  if (Serial && millis() - ultimaTraza >= 100 && Serial.availableForWrite() >= 60) {
    ultimaTraza = millis();
    Serial.print(F("PISO=")); Serial.print(analogRead(S_PISO_IZQ));
    Serial.print(','); Serial.print(analogRead(S_PISO_DER));
    Serial.print(F(" PWM=")); Serial.print(controlMovimiento.ordenIzquierda());
    Serial.print(','); Serial.print(controlMovimiento.ordenDerecha());
    Serial.print(F(" ENEMIGO_RAW="));
    Serial.print(digitalRead(S_FRONT_IZQ));
    Serial.print(digitalRead(S_FRONT_CEN));
    Serial.println(digitalRead(S_FRONT_DER));
  }
#endif
}
#endif
