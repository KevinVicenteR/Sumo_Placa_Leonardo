#ifndef DECISION_MOVIMIENTO_H
#define DECISION_MOVIMIENTO_H

enum class TipoAccion {
    EvadirBordeIzq,
    EvadirBordeDer,
    EvadirBordeAmbos,
    AtaqueFrontal,
    AjusteIzq,
    AjusteDer,
    CorregirIzq,
    CorregirDer,
    DefensaIzq,
    DefensaDer,
    Busqueda
};

struct DecisionMovimiento {
    TipoAccion tipo;
    bool enemigoFrente;
    bool cercaBorde;
    bool ataqueDirecto;
};

#endif
