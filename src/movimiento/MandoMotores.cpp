#include "movimiento/MandoMotores.h"

namespace {
long magnitud(long v) { return v < 0 ? -v : v; }
}

void MandoMotores::nuevoCiclo(unsigned long ahora, bool conRutina) {
    // Arranque suave: sin rutina de inicio, el avance sube desde 0 hasta
    // VelocidadAtaque durante TiempoArranqueSuave
    if (primerCiclo) inicioMovimiento = ahora;
    const unsigned long desdeInicio = ahora - inicioMovimiento;
    limiteArranque = !conRutina && rampa > 0 && TiempoArranqueSuave > 0 && desdeInicio < TiempoArranqueSuave
        ? (long)VelocidadAtaque * desdeInicio / TiempoArranqueSuave : 255;

    // Rampa: el paso permitido depende de los ms transcurridos desde el último ciclo
    const unsigned long dtRampa = primerCiclo ? 0 : ahora - ultimaRampa;
    primerCiclo = false;
    ultimaRampa = ahora;
    const unsigned long paso = dtRampa * (unsigned long)rampa;
    pasoRampa = paso > 255 ? 255 : (int)paso;

    estimarVelocidad(ahora);
}

// Acerca la orden actual al objetivo sin superar pasoRampa. Bajar la velocidad
// o cambiar de sentido para frenar no se limita (sería peligroso).
int MandoMotores::conRampa(int actual, int objetivo) const {
    const long producto = (long)actual * objetivo;
    if (objetivo == 0 || (producto > 0 && magnitud(objetivo) <= magnitud(actual))) {
        return objetivo;
    }
    const int base = producto > 0 ? actual : 0;
    const int diferencia = objetivo - base;
    if (diferencia > pasoRampa) return base + pasoRampa;
    if (diferencia < -pasoRampa) return base - pasoRampa;
    return objetivo;
}

void MandoMotores::mover(IMotor& motor, int izq, int der, bool urgente) {
    // 1. Limitar el avance (solo cuando las dos ruedas van hacia delante)
    const int mayor = izq > der ? izq : der;
    if (izq >= 0 && der >= 0 && mayor > 0) {
        int limite = limiteArranque;
        if (limites.salidaSuave < limite) limite = limites.salidaSuave;
        if (limites.cercaBorde && VelocidadCercaBorde < limite) limite = VelocidadCercaBorde;
        if (UsarEstimadorBorde && (!limites.empujando || EstimadorLimitaEmbestida) &&
            limites.estimador < limite) {
            limite = limites.estimador;
        }
        // Se reducen las dos ruedas en la misma proporción para no cambiar la curva
        if (mayor > limite) {
            izq = (long)izq * limite / mayor;
            der = (long)der * limite / mayor;
        }
    }
    // 2. Rampa (salvo en maniobras urgentes)
    if (rampa > 0 && !urgente) {
        izq = conRampa(ordenIzq, izq);
        der = conRampa(ordenDer, der);
    }
    // 3. Recordar y enviar
    ordenIzq = izq;
    ordenDer = der;
    motor.mover(izq, der);
}

// La rueda no alcanza la orden al instante: se filtra con la inercia TauRuedas
void MandoMotores::estimarVelocidad(unsigned long ahora) {
    const unsigned long dt = ahora - ultimaEstimacion;
    if (dt == 0) {
        return;
    }
    ultimaEstimacion = ahora;
    const long paso = dt >= TauRuedas ? 256 : (long)dt * 256 / TauRuedas;
    estIzq += ((long)ordenIzq * 256 - estIzq) * paso / 256;
    estDer += ((long)ordenDer * 256 - estDer) * paso / 256;
}

bool MandoMotores::calcularFreno(unsigned long duracionMaxima, Freno& freno) const {
    const long izq = estIzq / 256, der = estDer / 256;
    const long mayor = magnitud(izq) > magnitud(der) ? magnitud(izq) : magnitud(der);
    if (mayor < VelocidadMinimaFreno) {
        return false;
    }
    // Cada rueda frena en sentido contrario a su giro, y más tiempo cuanto más rápido iba
    freno.izq = -izq * VelocidadFreno / mayor;
    freno.der = -der * VelocidadFreno / mayor;
    freno.duracion = duracionMaxima * mayor / VelocidadMaxima;
    return true;
}
