#ifndef RETENCION_DETECCION_H
#define RETENCION_DETECCION_H

// Mantiene una detección activa durante un tiempo tras perderla,
// para tolerar parpadeos del sensor.
class RetencionDeteccion {
public:
    explicit RetencionDeteccion(unsigned long duracionMs): duracionMs(duracionMs) {}

    bool actualizar(bool detectado, unsigned long ahora) {
        if (detectado) {
            ultimaDeteccion = ahora;
            activo = true;
            return true;
        }
        if (activo && ahora - ultimaDeteccion < duracionMs) {
            return true;
        }
        activo = false;
        return false;
    }

private:
    unsigned long duracionMs;
    unsigned long ultimaDeteccion = 0;
    bool activo = false;
};

#endif
