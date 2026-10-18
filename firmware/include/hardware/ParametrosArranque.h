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
