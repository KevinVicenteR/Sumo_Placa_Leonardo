#ifndef DECISION_MOVIMIENTO_H
#define DECISION_MOVIMIENTO_H

enum class TipoAccion {
    EvadirBordeIzq,
    EvadirBordeDer,
    EvadirBordeAmbos,
    AtaqueFrontal,
    AjusteIzq,     // de frente pero algo hacia la izquierda (frontal + 45° izquierdo)
    AjusteDer,
    CorregirIzq,
    CorregirDer,
    DefensaIzq,
    DefensaDer,
    Busqueda
};

struct DecisionMovimiento {
    TipoAccion tipo;
    // El sensor frontal ve al enemigo (se informa también al evadir el borde).
    // Sin valor por defecto para seguir siendo agregado en C++11: {tipo} lo deja en false
    bool enemigoFrente;
};

#endif
