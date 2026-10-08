// ControlAtaque.h — Velocidad de ataque
// - Con el rival de frente, la velocidad sube de VelocidadAtaque a
//   VelocidadEmpuje durante TiempoEmbestida; al completarse, está "empujando".
// - Tras encontrar al rival con la rutina de inicio de las rondas 1 y 2,
//   embiste durante TiempoEmbestidaInicio, subiendo de VelocidadEmbestidaInicio
//   a VelocidadEmbestidaMaxima.
// - Con el rival en los tres sensores delanteros, VelocidadAtaqueDirecto.
// - Con el rival en dos o tres sensores delanteros, VelocidadAtaqueVariosFrontales
//   (sin subida gradual).
// - Ronda 3: al ver al rival por delante durante el avance lento, va contra él
//   acelerando poco a poco desde VelocidadAvanceInicio hasta VelocidadEmpuje en
//   TiempoAceleracionRound3 (aproximación), mientras lo siga atacando.
#ifndef CONTROL_ATAQUE_H
#define CONTROL_ATAQUE_H

#include "estrategia/DecisionMovimiento.h"

class ControlAtaque {
public:
    // Al principio de cada ciclo: ¿sigue el rival de frente? ¿ya está empujando?
    void actualizarContacto(const DecisionMovimiento& decision, bool evadiendo, unsigned long ahora);
    // Velocidad para la acción de ataque de este ciclo
    int velocidad(const DecisionMovimiento& decision, unsigned long ahora);

    // La rutina de inicio encontró al rival: embestida a fondo
    void iniciarEmbestida(unsigned long ahora);
    // Ronda 3: el avance lento encontró al rival por delante
    void iniciarAproximacion(unsigned long ahora);
    // Pierde el contacto (por ejemplo, al evadir el borde)
    void olvidarFrente() { viendoFrente = false; empujandoRival = false; }

    bool empujando() const { return empujandoRival; }

private:
    bool viendoFrente = false;
    unsigned long inicioFrente = 0;
    bool empujandoRival = false;
    // Embestida de la rutina de inicio
    bool embestidaInicio = false;
    unsigned long finEmbestidaInicio = 0;
    bool ataqueDeRutina = false;
    // Aproximación de la ronda 3
    bool aproximacion = false;
    unsigned long inicioAproximacion = 0;
};

#endif
