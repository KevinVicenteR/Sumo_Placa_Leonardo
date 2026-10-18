// Parametros.h — Todos los valores ajustables del robot
// Valores elegidos con simulacion/optimizar.py (batería de combates con todos los
// rounds, rivales y factores externos). Hay que comprobarlos en el robot real.
// Velocidades en PWM (0-255), tiempos en milisegundos y distancias en metros,
// salvo que se indique otra cosa. Los scripts de simulacion/ cambian estos
// valores por su nombre: si se renombra uno, hay que actualizar los scripts.
#ifndef PARAMETROS_H
#define PARAMETROS_H

// 1. Movimientos: cada uno tiene su archivo en movimiento/parametros/ (motores,
// ataque, giro lateral, evasión del borde, búsqueda, frenado y rutinas de
// inicio), incluidos al final de este archivo. Los modos de Rumania están en
// movimiento/rumania/.

// 2. Sensores de enemigo

// Polaridad: 0 = se aprende en modo diagnóstico midiendo sin objetos delante;
// 1 = detectan en HIGH; -1 = detectan en LOW.
constexpr int PolaridadSensoresEnemigo = 0;
// Polaridad usada en combate (nunca se recalibra con el rival delante).
// Un bit por sensor, en orden: lateral izq, 45° izq, central, 45° der, lateral
// der. 31 = los cinco detectan en HIGH.
constexpr int MascaraEnemigosActivosHigh = 31;
// Una detección se mantiene estos ms tras perderla (tolera parpadeos).
constexpr unsigned long TiempoRetencionEnemigo = 40;
// Una detección nueva solo cuenta tras verse estos ms seguidos (0 = al instante).
constexpr unsigned long ConfirmacionSensorEnemigo = 0;

// 3. Sensores de piso

// El negro se mide al empezar el combate. Se considera línea blanca cuando la
// lectura se aleja del negro más de este porcentaje (y como mínimo
// UmbralLineaMinimo), sin importar si el sensor sube o baja sobre el blanco.
constexpr int PorcentajeUmbralLinea = 15;
constexpr int UmbralLineaMinimo = 60;
constexpr int MuestrasCalibracionPiso = 64;
// Referencias de negro hasta que se calibra (la calibración las sustituye).
constexpr int NegroPisoIzquierdo = 994;
constexpr int NegroPisoDerecho = 972;
// Aviso de "cerca del borde": la lectura se aleja del negro este porcentaje del
// margen de línea. Mientras dura, el avance se limita a VelocidadCercaBorde.
constexpr int PorcentajeCercaBorde = 70;
constexpr int VelocidadCercaBorde = 100;

// 4. Tiempos generales

// Espera inicial del modo diagnóstico (el de combate usa el módulo de arranque).
constexpr unsigned long TiempoInicioReglamentario = 5000;

// 5. Estimador de borde
// Calcula dónde está el robot dentro del dohyo y limita la velocidad hacia
// delante a la que aún le deja frenar antes del borde, aunque los sensores de
// piso tarden en ver la línea.
constexpr bool UsarEstimadorBorde = true;
// true = también limita la embestida si prevé el borde cerca.
constexpr bool EstimadorLimitaEmbestida = true;
// Geometría: dohyo reglamentario de 77 cm de diámetro (línea incluida) y línea de 2,5 cm.
constexpr float RadioDohyo = 0.385f;
constexpr float AnchoLineaBorde = 0.025f;
constexpr float DistanciaSensorPiso = 0.078f;     // por delante del eje de ruedas (ruedas atrás)
constexpr float SeparacionSensoresPiso = 0.04f;   // a cada lado del centro
constexpr float TrochaRuedas = 0.085f;            // distancia entre ruedas
// Modelo de las ruedas: VelocidadRuedaMaxima (m/s) a PWM 255, parada por
// debajo de ZonaMuertaPwm y constante de tiempo TauRuedaEstimador (s).
constexpr float VelocidadRuedaMaxima = 0.53f;
constexpr int ZonaMuertaPwm = 40;
constexpr float TauRuedaEstimador = 0.06f;
// Al arrancar puede estar hasta a esta distancia del centro, con rumbo desconocido.
constexpr float RadioSalidaEstimado = 0.15f;
// Con rutina de inicio se sabe dónde empieza: rounds 1 y 2 junto al centro (a lo
// sumo a IncertSalidaCentro), round 3 a RadioSalidaRound3 del centro mirándolo
// (con IncertSalidaRound3 de error y IncertRumboRound3 rad de rumbo).
constexpr float IncertSalidaCentro = 0.06f;
constexpr float RadioSalidaRound3 = 0.295f;
constexpr float IncertSalidaRound3 = 0.03f;
constexpr float IncertRumboRound3 = 0.3f;
// Cuánto crece la incertidumbre: fracción de lo recorrido, de lo girado y
// deriva del rumbo (rad) por metro.
constexpr float ErrorDistancia = 0.3f;
constexpr float ErrorGiro = 0.25f;
constexpr float DerivaRumbo = 0.3f;
// Desplazamiento desconocido (m/s) por empujones: atacando, empujando y de lado.
constexpr float EmpujonAtaque = 0.15f;
constexpr float EmpujonContacto = 0.4f;
constexpr float EmpujonLateral = 0.2f;
// Al ver la línea con un solo sensor: ángulo típico respecto al radio (rad) y
// su incertidumbre; incertidumbre de la posición tras la medida.
constexpr float AnguloLineaLado = 0.6f;
constexpr float IncertAnguloLinea = 0.5f;
constexpr float IncertPosicionLinea = 0.02f;
// Peor caso: desviaciones típicas que se cubren y distancia mínima al borde.
constexpr float SigmasSeguridad = 1.0f;
constexpr float MargenBorde = 0.09f;
// Frenada: desaceleración (m/s²) y retardo hasta empezar a frenar (s).
constexpr float DesaceleracionFreno = 4.0f;
constexpr float RetardoReaccion = 0.04f;
// Nunca limita por debajo de esto (si no, podría quedarse quieto).
constexpr int VelocidadMinimaEstimador = 100;
// Solo se fía de la predicción con incertidumbres menores que estas.
constexpr float IncertPosicionConfiable = 0.08f;
constexpr float IncertRumboConfiable = 0.8f;

// 6. Estrategia

// true = usa la política aprendida en el simulador (TablaPolitica.h) en vez de
// las reglas escritas a mano.
constexpr bool UsarPoliticaAprendida = false;

// 7. Interruptores DIP (eligen la rutina o el modo de Rumania)
//   DIP1 DIP2  rutina
//   off  off   1: espalda con espalda -> media vuelta en el sitio
//   ON   off   2: lado a lado -> pivota hacia el lado donde un sensor lateral
//              ve al rival (DIP3 si no lo ve ninguno)
//   off  ON    3: enfrentados en los bordes -> avanza hacia el centro
//   ON   ON    4: salida libre (en cualquier parte del dohyo, sin posición
//              fija) -> gira en el sitio buscando al rival con todos los sensores
//   DIP3: lado del giro (off = derecha, ON = izquierda)
// Los giros terminan al ver al rival de frente (o al agotar su tiempo) y el
// avance al verlo con cualquier sensor. Ver la línea cancela la rutina.
// true = un interruptor en ON lee LOW (contra GND, con pull-up).
constexpr bool DipActivoBajo = true;
// Rumania: cada uno coloca su robot donde quiere. true = el estimador de borde
// no supone la posición de salida de los rounds (empieza sin conocerla).
constexpr bool ColocacionLibre = true;

// --- Protección contra detecciones falsas ---
// Una detección solo termina una acción si dura ConfirmacionDeteccion ms y
// nunca antes del tiempo mínimo de esa acción.
constexpr unsigned long ConfirmacionDeteccion = 5;

// Espera tras START en ms: 0 = arranque inmediato (Rumania).
constexpr unsigned long EsperaInicioCombate = 0;

// 8. Módulo de arranque
constexpr bool ModuloArranqueActivoAlto = true;
// RUN debe mantenerse estos ms seguidos para arrancar (descarta pulsos breves
// del módulo, por ejemplo al pulsar PROG en el control remoto).
constexpr unsigned long FiltroModuloArranqueMs = 30;
// STOP debe mantenerse estos ms seguidos para parar: más largo para que el
// infrarrojo del rival no detenga (y reinicie) el combate.
constexpr unsigned long FiltroParadaModuloMs = 150;

// Una parada más corta que esto no es del árbitro (para reiniciar hay que volver a
// colocar los robots) sino una interferencia: al volver RUN, el robot sigue el
// mismo combate sin recalibrar el piso (recalibrar sobre la línea blanca le
// haría ver borde en todas partes).
// Debe ser corto: con el control remoto (STOP, RESTART y START) un START antes
// de este tiempo se tomaría como la misma pelea y se saltaría la rutina del round.
constexpr unsigned long TiempoReanudarCombate = 500;
// true = una parada más larga que TiempoReanudarCombate (STOP o PROG del control)
// reinicia la placa entera, como apagarla y encenderla: no queda nada del combate
// anterior y el siguiente START lee los DIP y hace la rutina del modo elegido.
// La caja negra (EEPROM) se guarda antes y no se pierde.
constexpr bool ReiniciarTrasParada = true;
// Solo se reanuda un combate que llevaba al menos este tiempo en marcha. Al
// pulsar PROG el módulo da pulsos breves de RUN (mientras parpadea su LED): el
// START que se pulsa después debe empezar un combate nuevo, con su rutina.
constexpr unsigned long TiempoMinimoParaReanudar = 1000;
// PROG del control remoto sin STOP antes: el módulo da cortes breves de RUN
// mientras parpadea, que parecerían interferencias y reanudarían el combate
// anterior. Un segundo corte antes de este tiempo tras reanudar se toma como
// PROG: reinicia la placa entera, como si no se hubiera combatido ese round.
constexpr unsigned long VentanaProgramacionMs = 2000;
// Aviso de "listo" con los LED RX y TX de la placa (no ocupan pines): encendidos
// mientras espera el START con todo reiniciado. Tras un reinicio por STOP o PROG,
// antes parpadean ParpadeosListo veces (PeriodoParpadeoListo ms encendidos y
// otros tantos apagados). Se apagan al empezar el combate. false = sin aviso.
constexpr bool AvisoListoLed = true;
constexpr int ParpadeosListo = 3;
constexpr unsigned long PeriodoParpadeoListo = 150;

// Rutina (1 espalda, 2 lado, 3 frente, 4 salida libre) según DIP1 (bit 0) y DIP2 (bit 1).
constexpr int rutinaSegunInterruptores(int dip) {
    return (dip & 3) == 0 ? 1 : (dip & 3) == 1 ? 2 : (dip & 3) == 2 ? 3 : 4;
}

// Para probar sin depender de los DIP: -1 = la rutina la eligen los DIP (normal);
// 0 (ninguna), 1, 2, 3 o 4 = siempre esa rutina. LadoForzado: 0 = lo elige DIP3; 1 = derecha;
// -1 = izquierda.
constexpr int RutinaForzada = -1;
constexpr int LadoForzado = 0;

// Parámetros de cada movimiento, cada uno en su archivo (movimiento/parametros/)
#include "movimiento/parametros/ParametrosMotores.h"
#include "movimiento/parametros/ParametrosAtaque.h"
#include "movimiento/parametros/ParametrosGiroLateral.h"
#include "movimiento/parametros/ParametrosEvasion.h"
#include "movimiento/parametros/ParametrosBusqueda.h"
#include "movimiento/parametros/ParametrosFrenado.h"
#include "movimiento/parametros/ParametrosRutinasInicio.h"

#endif
