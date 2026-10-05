#ifndef CONFIGURACION_HARDWARE_H
#define CONFIGURACION_HARDWARE_H

#include "IConfiguracionHardware.H"

class ConfiguracionHardware: public IConfiguracionHardware {
public:
    void inicializarPines() const override;
    // Interruptores DIP en ON: bit 0 = DIP1, bit 1 = DIP2, bit 2 = DIP3
    int leerInterruptores() const;
};

#endif
