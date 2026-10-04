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
    assertAccionSimple(TipoAccion::CorregirIzq, VelocidadCurva, VelocidadAtaque);
    assertAccionSimple(TipoAccion::CorregirDer, VelocidadAtaque, VelocidadCurva);
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
    assertMovimiento(m, -VelocidadMaxima, -VelocidadMaxima);

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

// Al ver la línea avanzando a toda velocidad: freno de cada rueda -> retroceso -> freno -> giro
// (el freno dura en proporción a la velocidad: a VelocidadAtaque, no a la máxima)
static const unsigned long InicioRetroceso = TiempoFrenadoRuedas * VelocidadAtaque / VelocidadMaxima;
static const unsigned long InicioGiro = InicioRetroceso + TiempoRetroceso + TiempoFrenado;

void test_evadir_borde_izq_frena_retrocede_gira_derecha_y_avanza_sin_bloquear(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    g_delayCallCount = 0;

    // Venía atacando recto: frena las dos ruedas por igual
    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 1000);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t);
    assertMovimiento(m, -VelocidadMaxima, -VelocidadMaxima);

    // La línea ya no se ve, pero la maniobra continúa
    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioRetroceso);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioRetroceso + TiempoRetroceso - 1);
    assertRetrocede(m);

    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioRetroceso + TiempoRetroceso);
    assertMovimiento(m, 0, 0);

    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioGiro);
    assertGiroEvasion(m, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioGiro + TiempoGiroEvasion - 1);
    assertGiroEvasion(m, 1);

    // Evasión completa: el robot mira hacia el centro y la búsqueda empieza avanzando
    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioGiro + TiempoGiroEvasion);
    assertAvanceBusqueda(m);

    TEST_ASSERT_EQUAL_INT(0, g_delayCallCount);
}

void test_evadir_borde_der_gira_izquierda(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 0);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, t);
    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioGiro);
    assertGiroEvasion(m, -1);
}

void test_frenado_proporcional_a_lo_que_hacia_cada_rueda(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // En curva: la rueda más rápida frena a tope y la otra en proporción
    const unsigned long t = acelerar(c, m, TipoAccion::CorregirIzq, 0);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, t);
    assertMovimiento(m, -(long)VelocidadCurva * VelocidadMaxima / VelocidadAtaque, -VelocidadMaxima);
}

void test_quieto_no_frena_y_retrocede_directamente(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1000);
    assertRetrocede(m);
}

void test_evadir_borde_ambos_gira_hacia_el_ultimo_lado_del_enemigo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::CorregirIzq, 0);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, t);
    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioRetroceso + TiempoRetrocesoAmbos - 1);
    assertRetrocede(m);

    const unsigned long giro = t + InicioRetroceso + TiempoRetrocesoAmbos + TiempoFrenado;
    c.ejecutar({TipoAccion::Busqueda}, m, giro);
    assertGiroEvasion(m, -1);

    c.ejecutar({TipoAccion::Busqueda}, m, giro + TiempoGiroEvasionAmbos - 1);
    assertGiroEvasion(m, -1);
}

void test_linea_girando_en_el_sitio_gira_sin_retroceder(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // Al arrancar el robot está quieto y la búsqueda empieza girando a la derecha
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertGiroBusqueda(m, 1);

    // El sensor derecho (el que va por delante) toca la línea: gira al otro lado, sin retroceder
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1010);
    assertGiroEvasion(m, -1);

    // Nuevos toques durante el giro en el sitio no reinician la maniobra
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1010 + TiempoGiroEvasionEnSitio - 1);
    assertGiroEvasion(m, -1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1010 + TiempoGiroEvasionEnSitio);
    assertAvanceBusqueda(m);
}

void test_linea_durante_el_paro_frena_y_retrocede(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // Aún se desliza por la inercia del ataque: frena según lo que estima y retrocede
    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 1);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t + 5);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);

    // El freno dura a lo sumo lo que corresponde a VelocidadAtaque (InicioRetroceso)
    c.ejecutar({TipoAccion::Busqueda}, m, t + 5 + InicioRetroceso);
    assertRetrocede(m);
}

void test_linea_al_girar_tras_asentarse_gira_sin_retroceder(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // Tras atacar y el paro, busca girando en el sitio; la inercia ya pasó
    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 1);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 1 + TiempoParoPerdida);
    assertGiroBusqueda(m, 1);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t + 1 + TiempoParoPerdida + 2 * TauRuedas);
    assertGiroEvasion(m, 1);
}

void test_enemigo_no_aborta_el_giro_minimo_de_evasion(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t);
    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioGiro);
    assertGiroEvasion(m, 1);

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, t + InicioGiro + TiempoGiroEvasion - 1);
    assertGiroEvasion(m, 1);

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, t + InicioGiro + TiempoGiroEvasion);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
}

void test_enemigo_lateral_espera_fin_del_giro_de_evasion(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t);
    c.ejecutar({TipoAccion::Busqueda}, m, t + InicioGiro);

    c.ejecutar({TipoAccion::CorregirIzq}, m, t + InicioGiro + TiempoGiroEvasion);
    assertMovimiento(m, VelocidadCurva, VelocidadAtaque);
}

void test_tras_ataque_al_final_del_escape_busqueda_empieza_girando(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t0 = acelerar(c, m, TipoAccion::AtaqueFrontal, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t0);
    c.ejecutar({TipoAccion::Busqueda}, m, t0 + InicioGiro);
    const unsigned long t = t0 + InicioGiro + TiempoGiroEvasion;
    c.ejecutar({TipoAccion::CorregirDer}, m, t);

    // El enemigo se pierde tras atacar: la búsqueda reinicia con un giro
    c.ejecutar({TipoAccion::Busqueda}, m, t + 10);
    c.ejecutar({TipoAccion::Busqueda}, m, t + 10 + TiempoParoPerdida);
    assertGiroBusqueda(m, 1);
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

    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1150);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
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

void test_linea_interrumpe_el_giro_lateral(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::DefensaIzq, 1000);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t);
    // Frena solo lo que se movía: con rueda pivote, la rueda que empujaba
    if (GiroLateralEnRueda) {
        assertMovimiento(m, 0, -VelocidadMaxima);
    }
    c.ejecutar({TipoAccion::Busqueda}, m, t + TiempoFrenadoRuedas);
    assertRetrocede(m);
}

void test_enemigo_frontal_no_aborta_retroceso(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, t + InicioRetroceso + 10);
    assertRetrocede(m);
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
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);

    // Alineado de frente TiempoEmbestida: va con todo
    const unsigned long alineado = 1000 + TiempoEmbestida / 2 + TiempoEmbestida;
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, alineado);
    assertMovimiento(m, VelocidadEmpuje, VelocidadEmpuje);

    // Ya lanzado, sigue con todo aunque el enemigo pase a un sensor de 45°
    c.ejecutar({TipoAccion::CorregirDer}, m, alineado + 1);
    assertMovimiento(m, VelocidadEmpuje, VelocidadCurva * VelocidadEmpuje / VelocidadAtaque);
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

void test_linea_justo_tras_girar_gira_sin_retroceder_aunque_ya_avance(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;

    // Gira en el sitio y justo después se le ordena avanzar: por la inercia las
    // ruedas aún giran en sentidos opuestos, así que no debe retroceder
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TauRuedas);
    assertGiroBusqueda(m, 1);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, 1000 + TauRuedas + 1);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, 1000 + TauRuedas + 2);
    assertGiroEvasion(m, 1);
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

    DecisionMovimiento ataque = {TipoAccion::AtaqueFrontal, true, true};
    c.ejecutar(ataque, m, 1001);
    assertMovimiento(m, VelocidadCercaBorde, VelocidadCercaBorde);
}

void test_enemigo_espera_el_giro_completo() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    const unsigned long t = acelerar(c, m, TipoAccion::AtaqueFrontal, 0);
    c.ejecutar({TipoAccion::EvadirBordeIzq}, m, t);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, t + InicioGiro + TiempoMinimoGiroEvasion);
    assertGiroEvasion(m, 1);
    c.ejecutar({TipoAccion::AtaqueFrontal}, m, t + InicioGiro + TiempoGiroEvasion);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
}

void test_blanco_al_terminar_escape_en_sitio_no_avanza() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1010);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1010 + TiempoGiroEvasionEnSitio);
    assertMovimiento(m, 0, 0);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 2010);
    assertMovimiento(m, 0, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, 2011);
    assertAvanceBusqueda(m);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_enemigo_espera_el_giro_completo);
    RUN_TEST(test_blanco_al_terminar_escape_en_sitio_no_avanza);
    RUN_TEST(test_acciones_simples_velocidades_correctas);
    RUN_TEST(test_busqueda_inicial_gira_y_luego_avanza);
    RUN_TEST(test_busqueda_gira_hacia_el_ultimo_lado_del_enemigo);
    RUN_TEST(test_paro_en_seco_al_perder_al_enemigo_en_pleno_ataque);
    RUN_TEST(test_paro_se_cancela_si_vuelve_a_ver_al_enemigo);
    RUN_TEST(test_evadir_borde_izq_frena_retrocede_gira_derecha_y_avanza_sin_bloquear);
    RUN_TEST(test_evadir_borde_der_gira_izquierda);
    RUN_TEST(test_frenado_proporcional_a_lo_que_hacia_cada_rueda);
    RUN_TEST(test_quieto_no_frena_y_retrocede_directamente);
    RUN_TEST(test_evadir_borde_ambos_gira_hacia_el_ultimo_lado_del_enemigo);
    RUN_TEST(test_linea_girando_en_el_sitio_gira_sin_retroceder);
    RUN_TEST(test_linea_durante_el_paro_frena_y_retrocede);
    RUN_TEST(test_linea_al_girar_tras_asentarse_gira_sin_retroceder);
    RUN_TEST(test_enemigo_no_aborta_el_giro_minimo_de_evasion);
    RUN_TEST(test_enemigo_lateral_espera_fin_del_giro_de_evasion);
    RUN_TEST(test_tras_ataque_al_final_del_escape_busqueda_empieza_girando);
    RUN_TEST(test_giro_lateral_sigue_hasta_ver_al_enemigo_de_frente);
    RUN_TEST(test_giro_lateral_termina_por_tiempo);
    RUN_TEST(test_linea_interrumpe_el_giro_lateral);
    RUN_TEST(test_enemigo_frontal_no_aborta_retroceso);
    RUN_TEST(test_ajuste_centra_al_enemigo_sin_dejar_de_atacar);
    RUN_TEST(test_se_acomoda_y_luego_va_con_todo);
    RUN_TEST(test_el_empuje_empieza_de_cero_tras_perder_al_enemigo);
    RUN_TEST(test_linea_justo_tras_girar_gira_sin_retroceder_aunque_ya_avance);
    RUN_TEST(test_avance_de_busqueda_en_pulsos);
    RUN_TEST(test_cerca_del_borde_limita_tambien_el_ataque);
    return UNITY_END();
}
