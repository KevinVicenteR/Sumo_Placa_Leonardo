// IContextoCombate.h — Lo que la estrategia puede consultar del movimiento
// La red de lógica (EstrategiaRed) necesita saber algo más que los sensores:
// si el estimador ve el borde cerca por delante y si sigue la rutina de inicio.
// ControlMovimiento lo implementa; main.cpp los conecta.
#ifndef ICONTEXTO_COMBATE_H
#define ICONTEXTO_COMBATE_H

class IContextoCombate {
public:
    virtual ~IContextoCombate() {}
    // El estimador de borde limita el avance (el borde está cerca por delante)
    virtual bool bordeDelante() const = 0;
    // Sigue la rutina de inicio del round
    virtual bool enRutina() const = 0;
};

#endif
