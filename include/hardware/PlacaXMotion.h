// =============================================================================
// PlacaXMotion.h — Configuración de la placa JSumo XMotion
// -----------------------------------------------------------------------------
// Prepara todos los pines al encender y lee los interruptores DIP que eligen
// la rutina de inicio del round.
// =============================================================================
#ifndef PLACA_XMOTION_H
#define PLACA_XMOTION_H

class PlacaXMotion {
public:
    // Configura cada pin como entrada o salida, con los motores sin potencia
    void inicializarPines() const;
    // Interruptores DIP en ON: bit 0 = DIP1, bit 1 = DIP2, bit 2 = DIP3
    int leerInterruptores() const;
};

#endif
