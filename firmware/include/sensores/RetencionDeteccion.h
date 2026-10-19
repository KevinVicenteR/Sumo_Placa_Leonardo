#ifndef RETENCION_DETECCION_H
#define RETENCION_DETECCION_H

class RetencionDeteccion {
public:
    explicit RetencionDeteccion(unsigned long duracionMs, unsigned long confirmacionMs = 0)
        : duracionMs(duracionMs), confirmacionMs(confirmacionMs) {}

    bool actualizar(bool detectado, unsigned long ahora) {
        if (detectado) {
            if (!candidato) {
                candidato = true;
                inicioCandidato = ahora;
            }
            if (activo || ahora - inicioCandidato >= confirmacionMs) {
                ultimaDeteccion = ahora;
                activo = true;
                return true;
            }
            return false;
        }
        candidato = false;
        if (activo && ahora - ultimaDeteccion < duracionMs) {
            return true;
        }
        activo = false;
        return false;
    }

private:
    unsigned long duracionMs;
    unsigned long confirmacionMs;
    unsigned long ultimaDeteccion = 0;
    unsigned long inicioCandidato = 0;
    bool activo = false;
    bool candidato = false;
};

#endif
