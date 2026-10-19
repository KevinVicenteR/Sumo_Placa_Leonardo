// ParametrosArranque.h — Arranque: módulo de START, espera reglamentaria e interruptores DIP
// Lo incluye Parametros.h: los scripts de simulacion/ cambian estos valores
// por su nombre (--param). Velocidades en PWM (0-255), tiempos en ms y
// distancias en metros, salvo que se indique otra cosa.
#ifndef PARAMETROS_ARRANQUE_H
#define PARAMETROS_ARRANQUE_H

// --- Espera tras el START ---
// Espera inicial del modo diagnóstico (el de combate usa el módulo de arranque).
constexpr unsigned long TiempoInicioReglamentario = 5000;
// Reglamento SECIHTI: tras el START el robot espera este tiempo (ms) con los
// motores apagados antes de su primer movimiento (si sale antes, se repite el
// round). El piso se calibra al recibir START. 0 = sale al instante.
// Solo en el robot real: en el simulador los dos robots salen a la vez.
constexpr unsigned long EsperaInicioCombate = 5000;

// --- Interruptores DIP ---
// DIP1 y DIP2 eligen la ronda (movimiento/rondas/: Ronda1, Ronda2, Ronda3 y
// Libre) y DIP3 el lado (off = derecha, ON = izquierda).
// true = un interruptor en ON lee LOW (contra GND, con pull-up).
constexpr bool DipActivoBajo = true;
// Rumania: cada uno coloca su robot donde quiere. true = el estimador de borde
// no supone la posición de salida de los rounds (empieza sin conocerla).
constexpr bool ColocacionLibre = false;

// --- Módulo de arranque (control remoto) ---
constexpr bool ModuloArranqueActivoAlto = true;
// RUN debe mantenerse estos ms seguidos para arrancar (descarta pulsos breves
// del módulo, por ejemplo al pulsar PROG en el control remoto).
constexpr unsigned long FiltroModuloArranqueMs = 30;
// STOP debe mantenerse estos ms seguidos para parar: más largo para que el
// infrarrojo del rival no detenga (y reinicie) el combate.
constexpr unsigned long FiltroParadaModuloMs = 150;

// STOP continuo previo al flanco RUN: rechaza parpadeos de programación.
// 300 ms no añaden espera al START normal: el módulo ya llevaba en STOP.
constexpr unsigned long StopMinimoAntesDeStart = 300;
// 0 = no reanudar nunca: toda parada confirmada reinicia inmediatamente.
constexpr unsigned long TiempoReanudarCombate = 0;
// Se conserva el parámetro por compatibilidad; toda parada reinicia la placa.
constexpr bool ReiniciarTrasParada = true;
// Parámetros antiguos sin uso con la reanudación desactivada.
constexpr unsigned long TiempoMinimoParaReanudar = 1000;
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

#endif
