#ifndef PERCEPCION_H
#define PERCEPCION_H

#include "LecturasSensores.H"
#include "IPercepcion.H"

class Percepcion: public IPercepcion {
public:
    LecturasSensores leer() const override;
    LecturasSensores leerBorde() const override;
    // Llamar en setup() con el robot sobre el negro del dohyo.
    static void calibrarPiso();

private:
    static bool lecturaDigitalMayoritaria(int pin);
    static bool detectarLineaSeguro(uint8_t pin); 
    bool aplicarRetencion(uint8_t indice, bool detectado) const;
    uint8_t obtenerRetencion(uint8_t indice) const;
    void establecerRetencion(uint8_t indice, uint8_t valor) const;

    // Memoria de 32 bits requerida para la persistencia lateral
    mutable uint32_t retenciones = 0;
};

#endif
