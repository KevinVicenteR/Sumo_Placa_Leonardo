#include "Percepcion.H"
#include "Pines.H"

namespace {
    // -- CONFIGURACIÓN DE MEMORIA DE SENSORES (CICLOS) --
    // Aumenta estos valores si el robot pierde al enemigo muy rápido
    constexpr uint8_t MEMORIA_LATERAL = 15; // Mucha memoria para no perderlo al girar
    constexpr uint8_t MEMORIA_DIAGONAL = 10;
    constexpr uint8_t MEMORIA_FRONTAL = 0;  // Poca memoria para ser preciso al frente

    // -- SENSIBILIDAD DEL PISO --
    // El umbral se coloca en este porcentaje del camino entre el negro calibrado y
    // BLANCO. Más alto = detecta antes (más sensible), pero más riesgo de falsos.
    constexpr int SENSIBILIDAD_PISO_PCT = 50;
    constexpr uint8_t MUESTRAS_CALIBRACION = 16;
}

// Umbrales activos (BLANCO hasta que se calibre sobre el dohyo negro).
static int umbralPisoIzq = BLANCO;
static int umbralPisoDer = BLANCO;

// Variables estáticas para llevar la cuenta de la memoria de cada sensor
static uint8_t cuentaLatIzq = 0;
static uint8_t cuentaC45Izq = 0;
static uint8_t cuentaFrontal = 0;
static uint8_t cuentaC45Der = 0;
static uint8_t cuentaLatDer = 0;

bool Percepcion::lecturaDigitalMayoritaria(int pin) {
    // Leemos el enemigo (asumiendo lógica invertida: LOW = detectado)
    return digitalRead(pin) == LOW; 
}

static int calcularUmbral(uint8_t pin) {
    long suma = 0;
    for (uint8_t i = 0; i < MUESTRAS_CALIBRACION; ++i) suma += analogRead(pin);
    const int negro = static_cast<int>(suma / MUESTRAS_CALIBRACION);
    // Lectura dudosa (robot sobre blanco o sensor desconectado): umbral fijo.
    if (negro <= BLANCO * 2) return BLANCO;
    return BLANCO + static_cast<int>((static_cast<long>(negro - BLANCO) * SENSIBILIDAD_PISO_PCT) / 100);
}

void Percepcion::calibrarPiso() {
    umbralPisoIzq = calcularUmbral(S_PISO_IZQ);
    umbralPisoDer = calcularUmbral(S_PISO_DER);
}

bool Percepcion::detectarLineaSeguro(uint8_t pin) {
    // Blanco refleja más: lectura baja. Una sola muestra basta para reaccionar.
    return analogRead(pin) < (pin == S_PISO_IZQ ? umbralPisoIzq : umbralPisoDer);
}

LecturasSensores Percepcion::leerBorde() const {
    LecturasSensores lecturas{};
    // Leer siempre ambos: el control distingue un sensor (girar) de dos (retroceder).
    lecturas.lineaIzq = detectarLineaSeguro(S_PISO_IZQ);
    lecturas.lineaDer = detectarLineaSeguro(S_PISO_DER);
    if (lecturas.lineaIzq || lecturas.lineaDer) {
        cuentaLatIzq = cuentaC45Izq = cuentaFrontal = cuentaC45Der = cuentaLatDer = 0;
    }
    return lecturas;
}

LecturasSensores Percepcion::leer() const {
    LecturasSensores lecturas = leerBorde();

    // 1. LECTURA DE PISO (Prioridad absoluta, sin retardo)

    // Entregar el borde inmediatamente, sin leer ni retener al enemigo.
    if (lecturas.lineaIzq || lecturas.lineaDer) {
        cuentaLatIzq = cuentaC45Izq = cuentaFrontal = cuentaC45Der = cuentaLatDer = 0;
        return lecturas;
    }

    // 2. LECTURA DE ENEMIGO (Con Memoria / Filtro Antirrebote)
    
    // -- Lateral Izquierdo --
    if (lecturaDigitalMayoritaria(S_LAT_IZQ)) {
        cuentaLatIzq = MEMORIA_LATERAL; // Recarga la memoria al máximo
        lecturas.latIzq = true;
    } else {
        if (cuentaLatIzq > 0) {
            cuentaLatIzq--;
            lecturas.latIzq = true; // Sigue siendo verdadero artificialmente
        } else {
            lecturas.latIzq = false;
        }
    }

    // -- Diagonal Izquierdo (Frontal Izq) --
    if (lecturaDigitalMayoritaria(S_FRONT_IZQ)) {
        cuentaC45Izq = MEMORIA_DIAGONAL;
        lecturas.c45Izq = true;
    } else {
        if (cuentaC45Izq > 0) { cuentaC45Izq--; lecturas.c45Izq = true; } 
        else { lecturas.c45Izq = false; }
    }

    // -- Frontal Central --
    if (lecturaDigitalMayoritaria(S_FRONT_CEN)) {
        cuentaFrontal = MEMORIA_FRONTAL;
        lecturas.frontal = true;
    } else {
        if (cuentaFrontal > 0) { cuentaFrontal--; lecturas.frontal = true; } 
        else { lecturas.frontal = false; }
    }

    // -- Diagonal Derecho (Frontal Der) --
    if (lecturaDigitalMayoritaria(S_FRONT_DER)) {
        cuentaC45Der = MEMORIA_DIAGONAL;
        lecturas.c45Der = true;
    } else {
        if (cuentaC45Der > 0) { cuentaC45Der--; lecturas.c45Der = true; } 
        else { lecturas.c45Der = false; }
    }

    // -- Lateral Derecho --
    if (lecturaDigitalMayoritaria(S_LAT_DER)) {
        cuentaLatDer = MEMORIA_LATERAL;
        lecturas.latDer = true;
    } else {
        if (cuentaLatDer > 0) { cuentaLatDer--; lecturas.latDer = true; } 
        else { lecturas.latDer = false; }
    }

    return lecturas;
}

// Métodos de retención originales mantenidos por compatibilidad con Percepcion.H
bool Percepcion::aplicarRetencion(uint8_t indice, bool detectado) const { return detectado; }
uint8_t Percepcion::obtenerRetencion(uint8_t indice) const { return 0; }
void Percepcion::establecerRetencion(uint8_t indice, uint8_t valor) const {}
