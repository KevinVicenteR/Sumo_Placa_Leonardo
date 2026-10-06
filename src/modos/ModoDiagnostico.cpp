// Solo se compila en el entorno "diagnostico" (usa el monitor serie)
#if defined(MODO_DIAGNOSTICO)
#include <Arduino.h>
#include "modos/ModoDiagnostico.h"
#include "Pines.h"
#include "Parametros.h"

void ModoDiagnostico::iniciar() {
    placa.inicializarPines();
    motores.deshabilitar();
    Serial.begin(115200);
    delay(TiempoInicioReglamentario);
    calibrar();
}

void ModoDiagnostico::calibrar() {
    motores.deshabilitar();
    percepcion.calibrarPiso();
    percepcion.calibrarSensoresEnemigo();
    Serial.print(F("CALIBRADO negroIzq=")); Serial.print(percepcion.negroIzquierdo());
    Serial.print(F(" margenIzq=")); Serial.print(percepcion.margenIzquierdo());
    Serial.print(F(" negroDer=")); Serial.print(percepcion.negroDerecho());
    Serial.print(F(" margenDer=")); Serial.println(percepcion.margenDerecho());
}

void ModoDiagnostico::actualizar() {
    if (Serial.available() && (Serial.read() == 'c')) calibrar();
    const LecturasSensores l = percepcion.leer();
    const int dip = placa.leerInterruptores();

    // Piso: lectura cruda y si detecta línea
    Serial.print(F("pisoIzq=")); Serial.print(analogRead(S_PISO_IZQ));
    Serial.print(F(" pisoDer=")); Serial.print(analogRead(S_PISO_DER));
    Serial.print(F(" | lineaIzq=")); Serial.print(l.lineaIzq);
    Serial.print(F(" lineaDer=")); Serial.print(l.lineaDer);
    // Enemigo: nivel crudo de cada pin (lat izq, 45 izq, central, 45 der, lat der)
    Serial.print(F(" | RAW="));
    Serial.print(digitalRead(S_LAT_IZQ)); Serial.print(',');
    Serial.print(digitalRead(S_FRONT_IZQ)); Serial.print(',');
    Serial.print(digitalRead(S_FRONT_CEN)); Serial.print(',');
    Serial.print(digitalRead(S_FRONT_DER)); Serial.print(',');
    Serial.print(digitalRead(S_LAT_DER));
    // Módulo de arranque
    if (PIN_MODULO_ARRANQUE >= 0) {
        Serial.print(F(" | ARRANQUE=")); Serial.print(digitalRead((uint8_t)PIN_MODULO_ARRANQUE));
    }
    // Interruptores DIP
    Serial.print(F(" | DIP="));
    Serial.print(dip & 1 ? '1' : '0');
    Serial.print(dip & 2 ? '1' : '0');
    Serial.print(dip & 4 ? '1' : '0');
    // Enemigo ya filtrado (lo que usa la estrategia)
    Serial.print(F(" | VE="));
    Serial.print(l.latIzq); Serial.print(',');
    Serial.print(l.c45Izq); Serial.print(',');
    Serial.print(l.frontal); Serial.print(',');
    Serial.print(l.c45Der); Serial.print(',');
    Serial.println(l.latDer);
    delay(100);
}
#endif
