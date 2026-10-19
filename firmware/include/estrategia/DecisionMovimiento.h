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
    Busqueda,
    EsquivarIzq,
    EsquivarDer
};

struct DecisionMovimiento {
    TipoAccion tipo;
    bool enemigoFrente;
    bool cercaBorde;
    bool ataqueDirecto;
    int errorRival;
};

inline bool esBorde(TipoAccion tipo) {
    return tipo == TipoAccion::EvadirBordeIzq || tipo == TipoAccion::EvadirBordeDer ||
           tipo == TipoAccion::EvadirBordeAmbos;
}

inline bool esAtaque(TipoAccion tipo) {
    return tipo == TipoAccion::AtaqueFrontal || tipo == TipoAccion::AjusteIzq ||
           tipo == TipoAccion::AjusteDer || tipo == TipoAccion::CorregirIzq ||
           tipo == TipoAccion::CorregirDer;
}

inline bool esEsquiva(TipoAccion tipo) {
    return tipo == TipoAccion::EsquivarIzq || tipo == TipoAccion::EsquivarDer;
}

inline bool esDefensa(TipoAccion tipo) {
    return tipo == TipoAccion::DefensaIzq || tipo == TipoAccion::DefensaDer;
}

inline bool veDeFrente(const DecisionMovimiento& decision) {
    return decision.enemigoFrente || decision.tipo == TipoAccion::AtaqueFrontal ||
           decision.tipo == TipoAccion::AjusteIzq || decision.tipo == TipoAccion::AjusteDer;
}

#endif
