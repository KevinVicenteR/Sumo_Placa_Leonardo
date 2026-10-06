#include <unity.h>
#include "ControlMovimiento.H"
#include "Parametros.H"
#include "Arduino.h"

// Comportamiento clásico: sin rampa, búsqueda con giro en el sitio, escape recto
#define CLASICO 0, 0, false

class MotorMock : public IMotor {
public:
    int calls = 0;
    int izq = 0;
    int der = 0;

    void detener() override { mover(0, 0); }

    void mover(int velIzq, int velDer) override {
        izq = velIzq;
        der = velDer;
        calls++;
    }
};

// Rueda del lado del enemigo cuando lo ve solo un sensor de 45°
static int interior45(int ataque) {
    return Corregir45EnPivote ? 0 : VelocidadCurva * ataque / VelocidadAtaque;
}

static void assertMovimiento(MotorMock& m, int izq, int der) {
    TEST_ASSERT_EQUAL_INT(izq, m.izq);
    TEST_ASSERT_EQUAL_INT(der, m.der);
}

static void assertRetrocede(MotorMock& m) {
    assertMovimiento(m, -VelocidadRetroceso, -VelocidadRetroceso);
}

static void assertGiroEvasion(MotorMock& m, int sentido) {
    assertMovimiento(m, sentido * VelocidadGiroEvasion, -sentido * VelocidadGiroEvasion);
}

static void assertGiroBusqueda(MotorMock& m, int sentido) {
    assertMovimiento(m, sentido * VelocidadGiroBusqueda, -sentido * VelocidadGiroBusqueda);
}

static void assertAvanceBusqueda(MotorMock& m) {
    assertMovimiento(m, VelocidadAvance, VelocidadAvance);
}

static void assertAccionSimple(TipoAccion accion, int izq, int der) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    g_delayCallCount = 0;

    c.ejecutar({accion}, m, 0);

    TEST_ASSERT_EQUAL_INT(1, m.calls);
    assertMovimiento(m, izq, der);
    TEST_ASSERT_EQUAL_INT(0, g_delayCallCount);
}

void test_acciones_simples_velocidades_correctas(void) {
    assertAccionSimple(TipoAccion::AtaqueFrontal, VelocidadAtaque, VelocidadAtaque);
    // Visto solo a 45°: pivota sobre la rueda de ese lado (o curva)
    assertAccionSimple(TipoAccion::CorregirIzq, interior45(VelocidadAtaque), VelocidadAtaque);
    assertAccionSimple(TipoAccion::CorregirDer, VelocidadAtaque, interior45(VelocidadAtaque));
    if (GiroLateralEnRueda) {
        // La rueda del lado del enemigo se frena y hace de pivote
        assertAccionSimple(TipoAccion::DefensaIzq, 0, VelocidadRuedaPivote);
        assertAccionSimple(TipoAccion::DefensaDer, VelocidadRuedaPivote, 0);
    } else {
        assertAccionSimple(TipoAccion::DefensaIzq, -VelocidadPivoteLateral, VelocidadPivoteLateral);
        assertAccionSimple(TipoAccion::DefensaDer, VelocidadPivoteLateral, -VelocidadPivoteLateral);
    }
}

void test_busqueda_inicial_gira_y_luego_avanza(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertGiroBusqueda(m, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoGiroBusqueda - 1);
    assertGiroBusqueda(m, 1);

    // Pausa para que se asiente el giro antes de avanzar
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoGiroBusqueda);
    if (TiempoPausaBusqueda > 0) {
        assertMovimiento(m, 0, 0);
    }

    const unsigned long avance = 1000 + TiempoGiroBusqueda + TiempoPausaBusqueda;
    c.ejecutar({TipoAccion::Busqueda}, m, avance);
    assertAvanceBusqueda(m);

    // Otra pausa para frenar el avance antes de girar
    c.ejecutar({TipoAccion::Busqueda}, m, avance + TiempoAvanceBusqueda);
    if (TiempoPausaBusqueda > 0) {
        assertMovimiento(m, 0, 0);
    }

    c.ejecutar({TipoAccion::Busqueda}, m, avance + TiempoAvanceBusqueda + TiempoPausaBusqueda);
    assertGiroBusqueda(m, 1);
}

// Repite una acción el tiempo suficiente para que la velocidad estimada de las
// ruedas alcance la orden. Devuelve el instante final.
static unsigned long acelerar(ControlMovimiento& c, MotorMock& m, TipoAccion accion, unsigned long t) {
    c.ejecutar({accion}, m, t);
    c.ejecutar({accion}, m, t + TauRuedas);
    return t + TauRuedas;
}

void test_busqueda_gira_hacia_el_ultimo_lado_del_enemigo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // Pierde al enemigo que veía a la izquierda: tras el paro busca hacia ese lado
    const unsigned long t = acelerar(c, m, TipoAccion::CorregirIzq, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 1);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 1 + TiempoParoPerdida);
    assertGiroBusqueda(m, -1);

    // Tras ver al enemigo de lado, la búsqueda empieza al acabar el giro lateral
    c.ejecutar({TipoAccion::DefensaDer}, m, 2000);
    c.ejecutar({TipoAccion::Busqueda}, m, 2000 + TiempoMaxGiroLateral);
    assertGiroBusqueda(m, 1);
}

void test_paro_en_seco_al_perder_al_enemigo_en_pleno_ataque(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 1);
    const int freno = FrenoActivo ? -VelocidadFreno : 0;
    assertMovimiento(m, freno, freno);

    c.ejecutar({TipoAccion::Busqueda}, m, t + 1 + TiempoParoPerdida);
    assertGiroBusqueda(m, 1);
}

void test_paro_se_cancela_si_vuelve_a_ver_al_enemigo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 1);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, t + 10);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
}

static void assertGiroLateral(MotorMock& m, int sentido) {
    if (GiroLateralEnRueda) {
        assertMovimiento(m, sentido > 0 ? VelocidadRuedaPivote : 0, sentido > 0 ? 0 : VelocidadRuedaPivote);
    } else {
        assertMovimiento(m, sentido * VelocidadPivoteLateral, -sentido * VelocidadPivoteLateral);
    }
}

void test_giro_lateral_sigue_hasta_ver_al_enemigo_de_frente(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000);
    assertGiroLateral(m, -1);

    // Pasa por el hueco entre sensores y por el de 45°: el giro continúa
    c.ejecutar({TipoAccion::Busqueda}, m, 1050);
    assertGiroLateral(m, -1);
    c.ejecutar({TipoAccion::CorregirIzq}, m, 1100);
    assertGiroLateral(m, -1);

    // Una lectura frontal suelta no corta el giro; confirmada, sí
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1150);
    assertGiroLateral(m, -1);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1150 + ConfirmacionDeteccion);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
}

void test_giro_lateral_ignora_un_frontal_antes_del_giro_minimo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    c.ejecutar({TipoAccion::DefensaDer}, m, 1000);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1001);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1001 + ConfirmacionDeteccion);
    if (ConfirmacionDeteccion + 1 < TiempoMinimoGiroLateral) {
        assertGiroLateral(m, 1);
    }
}

void test_giro_lateral_termina_por_tiempo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    c.ejecutar({TipoAccion::DefensaDer}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxGiroLateral - 1);
    assertGiroLateral(m, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxGiroLateral);
    assertGiroBusqueda(m, 1);
}






void test_ajuste_centra_al_enemigo_sin_dejar_de_atacar(void) {
    assertAccionSimple(TipoAccion::AjusteIzq, VelocidadAtaque * PorcentajeAjuste / 100, VelocidadAtaque);
    assertAccionSimple(TipoAccion::AjusteDer, VelocidadAtaque, VelocidadAtaque * PorcentajeAjuste / 100);
}

void test_se_acomoda_y_luego_va_con_todo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // Visto solo a 45°: corrige, pero aún no está alineado
    c.ejecutar({TipoAccion::CorregirIzq}, m, 1000);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000 + TiempoEmbestida / 2);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000 + TiempoEmbestida);
    const int mitad = VelocidadAtaque + (VelocidadEmpuje - VelocidadAtaque) / 2;
    assertMovimiento(m, mitad, mitad);

    // Alineado de frente TiempoEmbestida: alcanza el techo
    const unsigned long alineado = 1000 + TiempoEmbestida / 2 + TiempoEmbestida;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, alineado);
    assertMovimiento(m, VelocidadEmpuje, VelocidadEmpuje);

    // Ya lanzado, sigue con todo aunque el enemigo pase a un sensor de 45°
    c.ejecutar({TipoAccion::CorregirDer}, m, alineado + 1);
    assertMovimiento(m, VelocidadEmpuje, interior45(VelocidadEmpuje));
    c.ejecutar({TipoAccion::AjusteDer}, m, alineado + 2);
    assertMovimiento(m, VelocidadEmpuje, VelocidadEmpuje * PorcentajeAjuste / 100);
}

void test_el_empuje_empieza_de_cero_tras_perder_al_enemigo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000);
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000 + TiempoEmbestida / 2);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000 + TiempoEmbestida);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
}



void test_avance_de_busqueda_en_pulsos(void) {
    if (TiempoPulsoAvance == 0) {
        return;
    }
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // Empieza girando; el avance llega tras el giro y la pausa
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    const unsigned long avance = 1000 + TiempoGiroBusqueda + TiempoPausaBusqueda;
    c.ejecutar({TipoAccion::Busqueda}, m, avance);
    assertAvanceBusqueda(m);
    c.ejecutar({TipoAccion::Busqueda}, m, avance + TiempoPulsoAvance);
    assertMovimiento(m, 0, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, avance + TiempoPulsoAvance + TiempoPausaPulso);
    assertAvanceBusqueda(m);
}

void test_cerca_del_borde_limita_tambien_el_ataque(void) {
    if (PorcentajeCercaBorde >= 100 || VelocidadCercaBorde >= VelocidadAtaque) {
        return;
    }
    ControlMovimiento c(CLASICO);
    MotorMock m;

    DecisionMovimiento lateral = {TipoAccion::DefensaDer, false, true};
    c.ejecutar(lateral, m, 1000);
    TEST_ASSERT_TRUE(m.izq <= VelocidadCercaBorde);

    // Termina el giro lateral (frontal confirmado tras el giro mínimo) y ataca limitado
    DecisionMovimiento ataque = {TipoAccion::AtaqueFrontal, true, true};
    c.ejecutar(ataque, m, 1000 + TiempoMinimoGiroLateral);
    c.ejecutar(ataque, m, 1000 + TiempoMinimoGiroLateral + ConfirmacionDeteccion);
    assertMovimiento(m, VelocidadCercaBorde, VelocidadCercaBorde);
}





void test_ataque_sube_gradualmente_y_borde_lo_interrumpe() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000 + TiempoEmbestida / 4);
    const int cuarto = VelocidadAtaque + (VelocidadEmpuje - VelocidadAtaque) / 4;
    assertMovimiento(m, cuarto, cuarto);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000 + TiempoEmbestida / 2);
    const int mitad = VelocidadAtaque + (VelocidadEmpuje - VelocidadAtaque) / 2;
    assertMovimiento(m, mitad, mitad);
    c.ejecutar({TipoAccion::EvadirBordeAmbos, true}, m, 1001 + TiempoEmbestida / 2);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
}

void test_un_sensor_retrocede_recto_y_gira_tras_separarse() {
    for (int lado = -1; lado <= 1; lado += 2) {
        ControlMovimiento c(CLASICO);
        MotorMock m;
        const TipoAccion borde = lado < 0 ? TipoAccion::EvadirBordeIzq : TipoAccion::EvadirBordeDer;
        c.ejecutar({borde}, m, 1000);
        assertMovimiento(m, -VelocidadRetrocesoUnSensor, -VelocidadRetrocesoUnSensor);
        c.ejecutar({borde}, m, 1100);
        assertMovimiento(m, -VelocidadRetrocesoUnSensor, -VelocidadRetrocesoUnSensor);
        c.ejecutar({TipoAccion::Busqueda}, m, 1200);
        const unsigned long paro = 1200 + TiempoSeparacionUnSensor;
        c.ejecutar({TipoAccion::Busqueda}, m, paro);
        assertMovimiento(m, 0, 0);
        const unsigned long giro = paro + TiempoFrenado;
        c.ejecutar({TipoAccion::AtaqueFrontal}, m, giro);
        assertGiroEvasion(m, -lado);
        c.ejecutar({TipoAccion::AtaqueFrontal}, m, giro + TiempoGiroEvasion);
        assertMovimiento(m, 0, 0);
        c.ejecutar({TipoAccion::AtaqueFrontal}, m, giro + TiempoGiroEvasion + TiempoAsentamientoEvasion);
        assertMovimiento(m, VelocidadSalidaUnSensor, VelocidadSalidaUnSensor);
        const unsigned long fin = giro + TiempoGiroEvasion + TiempoAsentamientoEvasion;
        c.ejecutar({TipoAccion::AtaqueFrontal}, m, fin + TiempoSalidaSuaveUnSensor - 1);
        assertMovimiento(m, VelocidadSalidaUnSensor, VelocidadSalidaUnSensor);
        c.ejecutar({TipoAccion::AtaqueFrontal}, m, fin + TiempoSalidaSuaveUnSensor);
        TEST_ASSERT_TRUE(m.izq > VelocidadSalidaUnSensor && m.izq == m.der);
        c.ejecutar({borde}, m, fin + TiempoSalidaSuaveUnSensor + 1);
        assertMovimiento(m, -VelocidadRetrocesoUnSensor, -VelocidadRetrocesoUnSensor);
    }
}
void test_busqueda_tras_un_sensor_sigue_hacia_el_lado_del_escape() {
    for (int sentido = -1; sentido <= 1; sentido += 2) {
        ControlMovimiento c(0);
        MotorMock m;
        // Recordar un enemigo al lado opuesto al escape.
        c.ejecutar({sentido > 0 ? TipoAccion::AjusteIzq : TipoAccion::AjusteDer}, m, 900);
        c.ejecutar({sentido > 0 ? TipoAccion::EvadirBordeIzq : TipoAccion::EvadirBordeDer}, m, 1000);
        c.ejecutar({TipoAccion::Busqueda}, m, 1200);
        const unsigned long paro = 1200 + TiempoSeparacionUnSensor;
        c.ejecutar({TipoAccion::Busqueda}, m, paro);
        c.ejecutar({TipoAccion::Busqueda}, m, paro + TiempoFrenado);
        c.ejecutar({TipoAccion::Busqueda}, m, paro + TiempoFrenado + TiempoGiroEvasion);
        const unsigned long fin = paro + TiempoFrenado + TiempoGiroEvasion + TiempoAsentamientoEvasion;
        c.ejecutar({TipoAccion::Busqueda}, m, fin);
        c.ejecutar({TipoAccion::Busqueda}, m, fin + TiempoArranqueBusqueda / 2);
        TEST_ASSERT_TRUE(sentido > 0 ? m.izq > m.der : m.der > m.izq);
    }
}
void test_segundo_sensor_cambia_a_recto_sin_reiniciar_limite() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1100);
    assertMovimiento(m, -VelocidadRetrocesoUnSensor, -VelocidadRetrocesoUnSensor);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1110);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1000 + TiempoMaximoRecuperacionBorde);
    assertMovimiento(m, 0, 0);
}
void test_ambos_retrocede_sin_reinicios_y_blanco_interrumpe_giro() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1000);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1000 + TiempoRetrocesoAmbos - 1);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoRetrocesoAmbos);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoRetrocesoAmbos + TiempoSeparacionBorde);
    assertMovimiento(m, 0, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoRetrocesoAmbos + TiempoSeparacionBorde + TiempoFrenado);
    assertGiroEvasion(m, 1);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1001 + TiempoRetrocesoAmbos + TiempoSeparacionBorde + TiempoFrenado);
    assertRetrocede(m);
}
void test_busqueda_default_en_curvas_sin_paradas_y_borde_la_interrumpe() {
    ControlMovimiento c(0);
    MotorMock m;
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertMovimiento(m, 0, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoArranqueBusqueda / 2);
    TEST_ASSERT_TRUE(m.izq > 0 && m.der > 0);
    TEST_ASSERT_TRUE(m.izq <= VelocidadAvance / 2 && m.der <= VelocidadAvance / 2);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoArcoBusqueda - 1);
    const int antesIzq = m.izq, antesDer = m.der;
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoArcoBusqueda);
    TEST_ASSERT_INT_WITHIN(1, antesIzq, m.izq);
    TEST_ASSERT_INT_WITHIN(1, antesDer, m.der);
    assertMovimiento(m, VelocidadAvance * PorcentajeArcoBusqueda / 100, VelocidadAvance);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1001 + TiempoArcoBusqueda);
    assertMovimiento(m, -VelocidadRetrocesoUnSensor, -VelocidadRetrocesoUnSensor);
}
void test_linea_persistente_no_reinicia_ni_vuelve_a_avanzar() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1000 + TiempoRetroceso);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1000 + TiempoMaximoRecuperacionBorde);
    assertMovimiento(m, 0, 0);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 5000);
    assertMovimiento(m, 0, 0);
}

void test_tres_frontales_cancelan_giro_y_pausa_pero_no_evasion() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    const DecisionMovimiento directo{TipoAccion::AtaqueFrontal, true, false, true};
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000);
    c.ejecutar(directo, m, 1010);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
    c.ejecutar({TipoAccion::Busqueda}, m, 1020);
    c.ejecutar(directo, m, 1030);
    TEST_ASSERT_TRUE(m.izq > 0 && m.izq == m.der);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1040);
    c.ejecutar(directo, m, 1050);
    assertRetrocede(m);
}

void test_arranque_no_dispara_aunque_tres_frontales_vean_enemigo() {
    ControlMovimiento c;
    MotorMock m;
    const DecisionMovimiento ataque{TipoAccion::AtaqueFrontal, true, false, true};
    c.ejecutar(ataque, m, 5000);
    assertMovimiento(m, 0, 0);
    c.ejecutar(ataque, m, 5000 + TiempoArranqueSuave / 4);
    TEST_ASSERT_TRUE(m.izq > 0 && m.izq <= VelocidadAtaque / 4);
    TEST_ASSERT_EQUAL(m.izq, m.der);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 5001 + TiempoArranqueSuave / 4);
    assertRetrocede(m);
}

void test_rutina_espalda_gira_hasta_ver_al_rival_y_frena_el_giro(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(1, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertMovimiento(m, VelocidadGiroInicio, -VelocidadGiroInicio);
    // Ve de lado durante el giro: sigue girando
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000 + TauRuedas);
    assertMovimiento(m, VelocidadGiroInicio, -VelocidadGiroInicio);
    // Un frontal antes del giro mínimo (un reflejo) no lo detiene
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, 1000 + TiempoMinimoGiroEspalda / 2);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, 1000 + TiempoMinimoGiroEspalda / 2 + ConfirmacionDeteccion);
    assertMovimiento(m, VelocidadGiroInicio, -VelocidadGiroInicio);
    // De frente y confirmado tras el mínimo: contragiro para no pasarse de largo
    const unsigned long visto = 1000 + TiempoMinimoGiroEspalda + 2 * TauRuedas;
    c.ejecutar({TipoAccion::Busqueda}, m, visto - 1);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto);
    assertMovimiento(m, VelocidadGiroInicio, -VelocidadGiroInicio);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto + ConfirmacionDeteccion);
    if (FrenoActivo) {
        TEST_ASSERT_TRUE(m.izq < 0 && m.der > 0);
    } else {
        assertMovimiento(m, 0, 0);
    }
    // Tras el freno, embiste
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto + ConfirmacionDeteccion + TiempoFrenadoRuedas + 1);
    assertMovimiento(m, VelocidadEmbestidaInicio, VelocidadEmbestidaInicio);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
}

void test_rutina_lado_gira_hacia_el_lado_del_rival(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(2, -1);
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000);
    // Pivota sobre la rueda izquierda: esa parada, la derecha empuja
    assertMovimiento(m, 0, VelocidadPivoteInicio);
    // Sin encontrarlo, termina al agotar su tiempo
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxGiroLado);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
}

void test_rutina_frente_avanza_hasta_ver_al_rival(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(3, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    TEST_ASSERT_TRUE(m.izq > 0 && m.izq == m.der);
    // Una detección al principio no corta el avance
    c.ejecutar({TipoAccion::CorregirIzq}, m, 1010);
    c.ejecutar({TipoAccion::CorregirIzq}, m, 1010 + ConfirmacionDeteccion);
    TEST_ASSERT_EQUAL_INT(3, c.rutinaActual());
    // Confirmada tras el mínimo, sí
    c.ejecutar({TipoAccion::CorregirIzq}, m, 1000 + TiempoMinimoAvanceInicio + ConfirmacionDeteccion);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
}

void test_la_linea_cancela_la_rutina(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(3, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1010);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    TEST_ASSERT_TRUE(m.izq <= 0 && m.der <= 0);
}

void test_sin_interruptores_la_rutina_es_la_del_round_1(void) {
    TEST_ASSERT_EQUAL_INT(1, rutinaSegunInterruptores(0));
    TEST_ASSERT_EQUAL_INT(2, rutinaSegunInterruptores(1));
    TEST_ASSERT_EQUAL_INT(3, rutinaSegunInterruptores(2));
    TEST_ASSERT_EQUAL_INT(0, rutinaSegunInterruptores(3));
    TEST_ASSERT_EQUAL_INT(1, rutinaSegunInterruptores(4));  // DIP3 solo elige el lado
}

void test_rutina_frente_ignora_los_laterales(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(3, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    // Algo fuera del dohyo visto de lado: sigue avanzando
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000 + TiempoMinimoAvanceInicio);
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000 + TiempoMinimoAvanceInicio + 2 * ConfirmacionDeteccion);
    TEST_ASSERT_EQUAL_INT(3, c.rutinaActual());
    TEST_ASSERT_TRUE(m.izq > 0 && m.izq == m.der);
}

void test_rutina_lado_elige_el_lado_con_el_sensor_lateral(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    // El DIP3 dice izquierda, pero el lateral derecho ve al rival: pivota a la derecha
    c.iniciarRutina(2, -1);
    c.ejecutar({TipoAccion::DefensaDer}, m, 1000);
    assertMovimiento(m, VelocidadPivoteInicio, 0);
    // Una vez decidido, no cambia de lado aunque luego lo vea el otro lateral
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1010);
    assertMovimiento(m, VelocidadPivoteInicio, 0);
}

void test_rutina_lado_sin_lateral_usa_el_dip3(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(2, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertMovimiento(m, VelocidadPivoteInicio, 0);
}

void test_visto_a_45_pivota_sobre_la_rueda_de_ese_lado(void) {
    if (!Corregir45EnPivote) {
        return;
    }
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar({TipoAccion::CorregirIzq}, m, 1000);
    assertMovimiento(m, 0, VelocidadAtaque);
    c.ejecutar({TipoAccion::CorregirDer}, m, 1001);
    assertMovimiento(m, VelocidadAtaque, 0);
}

void test_round_1_mantiene_su_velocidad_tras_la_embestida(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(1, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    const unsigned long visto = 1000 + TiempoMinimoGiroEspalda + 2 * TauRuedas;
    c.ejecutar({TipoAccion::Busqueda}, m, visto - 1);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto + ConfirmacionDeteccion);
    // Acabada la embestida, sigue empujando a la velocidad de siempre, no al máximo
    const unsigned long despues = visto + ConfirmacionDeteccion + TiempoEmbestidaInicio + TiempoEmbestida + 10;
    for (unsigned long t = visto + ConfirmacionDeteccion + 1; t <= despues; t += 10) {
        c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, t);
    }
    TEST_ASSERT_TRUE(m.izq <= VelocidadAtaqueRound12 && m.izq > 0);
    // Al perder al rival y volver a encontrarlo, ataque normal
    c.ejecutar({TipoAccion::Busqueda}, m, despues + 10);
    c.ejecutar({TipoAccion::Busqueda}, m, despues + 10 + TiempoParoPerdida + 10);
    for (unsigned long t = despues + 400; t <= despues + 400 + TiempoEmbestida + 20; t += 10) {
        c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, t);
    }
    TEST_ASSERT_EQUAL_INT(VelocidadEmpuje, m.izq);
}

void test_rutina_frente_espera_quieto_tras_avanzar(void) {
    if (!EsperarRound3) {
        return;
    }
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(3, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoAvanceInicio + 1);
    assertMovimiento(m, 0, 0);
    TEST_ASSERT_EQUAL_INT(3, c.rutinaActual());
    // Esperando, un lateral confirmado sí termina la rutina
    c.ejecutar({TipoAccion::DefensaDer}, m, 1000 + TiempoAvanceInicio + 10);
    c.ejecutar({TipoAccion::DefensaDer}, m, 1000 + TiempoAvanceInicio + 10 + ConfirmacionDeteccion);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
}

void test_reiniciar_vuelve_al_estado_inicial(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(1, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1010);
    c.reiniciar();
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    TEST_ASSERT_EQUAL_INT(0, c.ordenIzquierda());
    // Tras reiniciar, una nueva rutina arranca normalmente
    c.iniciarRutina(1, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 5000);
    assertMovimiento(m, VelocidadGiroInicio, -VelocidadGiroInicio);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_busqueda_tras_un_sensor_sigue_hacia_el_lado_del_escape);
    RUN_TEST(test_segundo_sensor_cambia_a_recto_sin_reiniciar_limite);
    RUN_TEST(test_arranque_no_dispara_aunque_tres_frontales_vean_enemigo);
    RUN_TEST(test_tres_frontales_cancelan_giro_y_pausa_pero_no_evasion);
    RUN_TEST(test_un_sensor_retrocede_recto_y_gira_tras_separarse);
    RUN_TEST(test_ambos_retrocede_sin_reinicios_y_blanco_interrumpe_giro);
    RUN_TEST(test_busqueda_default_en_curvas_sin_paradas_y_borde_la_interrumpe);
    RUN_TEST(test_linea_persistente_no_reinicia_ni_vuelve_a_avanzar);
    RUN_TEST(test_ataque_sube_gradualmente_y_borde_lo_interrumpe);
    RUN_TEST(test_acciones_simples_velocidades_correctas);
    RUN_TEST(test_busqueda_inicial_gira_y_luego_avanza);
    RUN_TEST(test_busqueda_gira_hacia_el_ultimo_lado_del_enemigo);
    RUN_TEST(test_paro_en_seco_al_perder_al_enemigo_en_pleno_ataque);
    RUN_TEST(test_paro_se_cancela_si_vuelve_a_ver_al_enemigo);
    RUN_TEST(test_giro_lateral_sigue_hasta_ver_al_enemigo_de_frente);
    RUN_TEST(test_giro_lateral_termina_por_tiempo);
    RUN_TEST(test_ajuste_centra_al_enemigo_sin_dejar_de_atacar);
    RUN_TEST(test_se_acomoda_y_luego_va_con_todo);
    RUN_TEST(test_el_empuje_empieza_de_cero_tras_perder_al_enemigo);
    RUN_TEST(test_avance_de_busqueda_en_pulsos);
    RUN_TEST(test_cerca_del_borde_limita_tambien_el_ataque);
    RUN_TEST(test_rutina_espalda_gira_hasta_ver_al_rival_y_frena_el_giro);
    RUN_TEST(test_rutina_lado_gira_hacia_el_lado_del_rival);
    RUN_TEST(test_rutina_frente_avanza_hasta_ver_al_rival);
    RUN_TEST(test_la_linea_cancela_la_rutina);
    RUN_TEST(test_sin_interruptores_la_rutina_es_la_del_round_1);
    RUN_TEST(test_giro_lateral_ignora_un_frontal_antes_del_giro_minimo);
    RUN_TEST(test_rutina_frente_ignora_los_laterales);
    RUN_TEST(test_rutina_lado_elige_el_lado_con_el_sensor_lateral);
    RUN_TEST(test_rutina_lado_sin_lateral_usa_el_dip3);
    RUN_TEST(test_visto_a_45_pivota_sobre_la_rueda_de_ese_lado);
    RUN_TEST(test_round_1_mantiene_su_velocidad_tras_la_embestida);
    RUN_TEST(test_rutina_frente_espera_quieto_tras_avanzar);
    RUN_TEST(test_reiniciar_vuelve_al_estado_inicial);
    return UNITY_END();
}
