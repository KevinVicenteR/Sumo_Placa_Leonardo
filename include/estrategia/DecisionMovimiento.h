// DecisionMovimiento.h — Qué ha decidido hacer la estrategia
// -----------------------------------------------------------------------------
// La estrategia elige un tipo de acción; el control de movimiento lo convierte
// en velocidades de motor.
#ifndef DECISION_MOVIMIENTO_H
#define DECISION_MOVIMIENTO_H

enum class TipoAccion {
    EvadirBordeIzq,     // línea bajo el sensor de piso izquierdo
    EvadirBordeDer,     // línea bajo el sensor de piso derecho
    EvadirBordeAmbos,   // línea bajo los dos sensores de piso
    AtaqueFrontal,      // rival de frente: avanzar recto
    AjusteIzq,          // rival de frente y algo a la izquierda
    AjusteDer,          // rival de frente y algo a la derecha
    CorregirIzq,        // rival solo en el sensor de 45° izquierdo
    CorregirDer,        // rival solo en el sensor de 45° derecho
    DefensaIzq,         // rival en el sensor lateral izquierdo
    DefensaDer,         // rival en el sensor lateral derecho
    Busqueda            // no se ve al rival
};

struct DecisionMovimiento {
    TipoAccion tipo;
    bool enemigoFrente;   // el sensor central ve al rival
    bool cercaBorde;      // un sensor de piso empieza a ver claro
    bool ataqueDirecto;   // los tres sensores delanteros ven al rival
};

// --- Clasificación de acciones (la usan varias partes del movimiento) ---

inline bool esBorde(TipoAccion tipo) {
    return tipo == TipoAccion::EvadirBordeIzq || tipo == TipoAccion::EvadirBordeDer ||
           tipo == TipoAccion::EvadirBordeAmbos;
}

// Acciones que avanzan hacia el rival
inline bool esAtaque(TipoAccion tipo) {
    return tipo == TipoAccion::AtaqueFrontal || tipo == TipoAccion::AjusteIzq ||
           tipo == TipoAccion::AjusteDer || tipo == TipoAccion::CorregirIzq ||
           tipo == TipoAccion::CorregirDer;
}

inline bool esDefensa(TipoAccion tipo) {
    return tipo == TipoAccion::DefensaIzq || tipo == TipoAccion::DefensaDer;
}

// El rival está delante del robot
inline bool veDeFrente(const DecisionMovimiento& decision) {
    return decision.enemigoFrente || decision.tipo == TipoAccion::AtaqueFrontal ||
           decision.tipo == TipoAccion::AjusteIzq || decision.tipo == TipoAccion::AjusteDer;
}

#endif
