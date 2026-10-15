#include <unity.h>
#include "movimiento/ControlMovimiento.h"
#include "movimiento/RedControl.h"
#include <string.h>
#include <vector>
#include "Parametros.h"
#include "Arduino.h"

// Unity ejecuta tearDown incluso si una aserción aborta la prueba con longjmp.
// Cada caso conserva los pesos entrenados para las pruebas de integración.
static std::vector<int8_t> pesosRedOriginales;
void setUp(void) {
    pesosRedOriginales.assign(redcontrol::Parametros,
                             redcontrol::Parametros + redcontrol::numParametros());
}
void tearDown(void) {
    memcpy(redcontrol::Parametros, pesosRedOriginales.data(), pesosRedOriginales.size());
}

static void neutralizarRedControl() {
    memset(redcontrol::Parametros, 0, redcontrol::numParametros());
}

// Comportamiento clásico: sin rampa y búsqueda con giro en el sitio
#define CLASICO 0, 0

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

// El robot no avanza (gira en el sitio, retrocede o está parado)
static void assertNoAvanza(MotorMock& m) {
    TEST_ASSERT_FALSE(m.izq > 0 && m.der > 0);
}

// La evasión solo retrocede si el robot llega a la línea avanzando: avanza
// recto los 300 ms anteriores a "hasta"
static void venirAvanzando(ControlMovimiento& c, MotorMock& m, unsigned long hasta) {
    for (unsigned long t = hasta - 300; t < hasta; t += 10) {
        c.ejecutar({TipoAccion::AtaqueFrontal}, m, t);
    }
}

// Giro del round 2 hacia el lado del rival: pivote sobre una rueda o en el sitio
// Giro del round 1 (lado 1 = derecha): media vuelta en el sitio
static void assertGiroRound1(MotorMock& m, int lado) {
    assertMovimiento(m, lado * VelocidadGiroInicio, -lado * VelocidadGiroInicio);
}

static void assertGiroRound2(MotorMock& m, int lado) {
    if (PivoteRound2) {
        assertMovimiento(m, lado > 0 ? VelocidadPivoteInicio : 0, lado > 0 ? 0 : VelocidadPivoteInicio);
    } else {
        assertMovimiento(m, lado * VelocidadGiroRound2, -lado * VelocidadGiroRound2);
    }
}

// Retroceso extra de la evasión por llegar rápido a la línea (llamar justo
// después del ciclo en que ve la línea)
static unsigned long retrocesoExtra(const ControlMovimiento& c) {
    const int v = c.velocidadAvanceEstimada();
    return v > 0 ? RetrocesoExtraAVelocidadMaxima * v / 255 : 0;
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
    // Visto solo a 45°: pivota sobre la rueda de ese lado (o curva); con PID lo
    // decide el error (ver test_pid_*)
    if (!UsarPID) {
        assertAccionSimple(TipoAccion::CorregirIzq, interior45(VelocidadAtaque), VelocidadAtaque);
        assertAccionSimple(TipoAccion::CorregirDer, VelocidadAtaque, interior45(VelocidadAtaque));
    }
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
    assertMovimiento(m, -VelocidadFreno, -VelocidadFreno);

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
    if (UsarPID) {
        return;   // con PID el ajuste lo calcula el regulador (ver test_pid_*)
    }
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
    if (UsarPID) {
        return;
    }
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
    // Pivotando sobre una rueda avanza y se limita; girando en el sitio no avanza
    if (GiroLateralEnRueda) {
        TEST_ASSERT_TRUE(m.izq <= VelocidadCercaBorde);
    }

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

// Avanza la evasión sin ver nada hasta que empieza a girar; devuelve ese instante
static unsigned long hastaGiroEvasion(ControlMovimiento& c, MotorMock& m, unsigned long t) {
    while (!(m.izq != 0 && m.izq == -m.der)) {
        t += 5;
        c.ejecutar({TipoAccion::Busqueda}, m, t);
        TEST_ASSERT_TRUE(t < 5000);
    }
    return t;
}

void test_ver_al_rival_corta_la_evasion(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    venirAvanzando(c, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1000);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
    // Retrocediendo junto a la línea no la corta aunque vea al rival
    for (unsigned long t = 1010; t <= 1060; t += 5) {
        c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, t);
    }
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
    // Ya separado y girando: un reflejo breve no la corta...
    const unsigned long giro = hastaGiroEvasion(c, m, 1060);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, giro + 5);
    TEST_ASSERT_TRUE(m.izq == -m.der);
    // ...pero verlo TiempoConfirmarRivalEnEvasion ms seguidos sí: va contra él
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, giro + 5 + TiempoConfirmarRivalEnEvasion);
    TEST_ASSERT_TRUE(m.izq > 0 && m.izq == m.der);

    // Vuelve a la línea enseguida: esta evasión ya no se corta (no se queda
    // atrapado en el borde cortando evasiones)
    const unsigned long otra = giro + 100;
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, otra);
    const unsigned long giro2 = hastaGiroEvasion(c, m, otra);
    TEST_ASSERT_TRUE(giro2 - giro < EsperaEntreCortesEvasion);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, giro2 + 5);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, giro2 + 5 + TiempoConfirmarRivalEnEvasion);
    TEST_ASSERT_TRUE(m.izq == -m.der);
}

void test_un_sensor_retrocede_recto_y_gira_tras_separarse() {
    for (int lado = -1; lado <= 1; lado += 2) {
        ControlMovimiento c(CLASICO);
        MotorMock m;
        const TipoAccion borde = lado < 0 ? TipoAccion::EvadirBordeIzq : TipoAccion::EvadirBordeDer;
        venirAvanzando(c, m, 1000);
        c.ejecutar({borde}, m, 1000);
        const unsigned long extra = retrocesoExtra(c);
        assertMovimiento(m, -VelocidadRetrocesoUnSensor, -VelocidadRetrocesoUnSensor);
        c.ejecutar({borde}, m, 1100);
        assertMovimiento(m, -VelocidadRetrocesoUnSensor, -VelocidadRetrocesoUnSensor);
        c.ejecutar({TipoAccion::Busqueda}, m, 1200 + extra);
        const unsigned long paro = 1200 + extra + TiempoSeparacionUnSensor;
        c.ejecutar({TipoAccion::Busqueda}, m, paro);
        assertMovimiento(m, 0, 0);
        const unsigned long giro = paro + TiempoFrenado;
        c.ejecutar({TipoAccion::Busqueda}, m, giro);
        assertGiroEvasion(m, -lado);
        c.ejecutar({TipoAccion::Busqueda}, m, giro + TiempoGiroEvasion);
        assertMovimiento(m, 0, 0);
        // El rival aparece justo al acabar la maniobra (aún sin confirmar)
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
        const unsigned long extra = retrocesoExtra(c);
        c.ejecutar({TipoAccion::Busqueda}, m, 1200 + extra);
        const unsigned long paro = 1200 + extra + TiempoSeparacionUnSensor;
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
    venirAvanzando(c, m, 1000);
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
    venirAvanzando(c, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1000);
    const unsigned long retroceso = TiempoRetrocesoAmbos + retrocesoExtra(c);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1000 + retroceso - 1);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + retroceso);
    assertRetrocede(m);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + retroceso + TiempoSeparacionBorde);
    assertMovimiento(m, 0, 0);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + retroceso + TiempoSeparacionBorde + TiempoFrenado);
    assertGiroEvasion(m, 1);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1001 + retroceso + TiempoSeparacionBorde + TiempoFrenado);
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
    assertNoAvanza(m);
}
// Mucho tiempo sobre la línea: nunca avanza hacia ella y nunca se queda quieto
// más de TiempoMaxParadoEnLinea (vuelve a intentar salir girando o retrocediendo)
void test_linea_persistente_no_se_queda_parado_ni_avanza() {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    venirAvanzando(c, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeDer}, m, 1000 + TiempoRetroceso);
    TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
    unsigned long paradoDesde = 0;
    bool parado = false;
    bool volvioAMoverse = false;
    for (unsigned long t = 1000 + TiempoRetroceso; t < 8000; t += 10) {
        c.ejecutar({TipoAccion::EvadirBordeDer}, m, t);
        assertNoAvanza(m);
        if (m.izq == 0 && m.der == 0) {
            if (!parado) { parado = true; paradoDesde = t; }
            TEST_ASSERT_TRUE(t - paradoDesde <= TiempoMaxParadoEnLinea + 10);
        } else {
            if (parado) volvioAMoverse = true;
            parado = false;
        }
    }
    TEST_ASSERT_TRUE(volvioAMoverse);
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
    venirAvanzando(c, m, 1340);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1340);
    c.ejecutar(directo, m, 1350);
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
    // Va tan despacio que no cuenta como "venía avanzando": sale girando
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 5001 + TiempoArranqueSuave / 4);
    assertNoAvanza(m);
}

void test_rutina_espalda_gira_hasta_ver_al_rival_y_frena_el_giro(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(1, 1);

    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    if (TiempoEsquivaRound1 > 0) {
        // Primero se separa del rival (están pegados) avanzando recto
        assertMovimiento(m, VelocidadEsquiva, VelocidadEsquiva);
    }
    // El giro empieza al terminar la esquiva
    const unsigned long g = 1000 + TiempoEsquivaRound1;
    c.ejecutar({TipoAccion::Busqueda}, m, g);
    assertGiroRound1(m, 1);
    // Lo ve con el lateral del lado hacia el que gira: sigue girando igual
    c.ejecutar({TipoAccion::DefensaDer}, m, g + TauRuedas);
    assertGiroRound1(m, 1);
    // Un frontal antes del giro mínimo (un reflejo) no lo detiene
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, g + TiempoMinimoGiroEspalda / 2);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, g + TiempoMinimoGiroEspalda / 2 + ConfirmacionDeteccion);
    assertGiroRound1(m, 1);
    // De frente y confirmado tras el mínimo: contragiro para no pasarse de largo
    const unsigned long visto = g + TiempoMinimoGiroEspalda + 2 * TauRuedas;
    c.ejecutar({TipoAccion::Busqueda}, m, visto - 1);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto);
    assertGiroRound1(m, 1);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto + ConfirmacionDeteccion);
    // Frena el giro con un contragiro
    TEST_ASSERT_TRUE(m.izq < 0 && m.der > 0);
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
    if (TiempoEsquivaRound2 > 0) {
        // Primero esquiva la embestida avanzando recto
        assertMovimiento(m, VelocidadEsquiva, VelocidadEsquiva);
        c.ejecutar({TipoAccion::DefensaIzq}, m, 1000 + TiempoEsquivaRound2);
    }
    // Gira hacia la izquierda (pivote sobre la rueda izquierda o en el sitio)
    assertGiroRound2(m, -1);
    // Sin encontrarlo, termina al agotar su tiempo
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoEsquivaRound2 + TiempoMaxGiroLado);
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

void test_rutina_frente_respeta_potencia_configurada(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(3, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    // Lo ve de frente: con aceleración cero debe atacar a potencia máxima.
    const unsigned long visto = 1000 + TiempoMinimoAvanceInicio;
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto);
    c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, visto + ConfirmacionDeteccion);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    if (TiempoAceleracionRound3 == 0) {
        assertMovimiento(m, VelocidadEmpuje, VelocidadEmpuje);
        // La potencia máxima conserva la prioridad del borde.
        c.ejecutar({TipoAccion::EvadirBordeAmbos, true}, m, visto + ConfirmacionDeteccion + 10);
        TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0);
        return;
    }
    TEST_ASSERT_TRUE(m.izq > 0 && m.izq <= VelocidadAvanceInicio + 5 && m.izq == m.der);
    // A mitad de la aceleración va más rápido, pero aún no a fondo
    int antes = m.izq;
    for (unsigned long t = visto + 10; t <= visto + TiempoAceleracionRound3 / 2; t += 10) {
        c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, t);
    }
    TEST_ASSERT_TRUE(m.izq > antes && m.izq < VelocidadEmpuje);
    // Sigue subiendo, sin pasar nunca del ataque normal
    antes = m.izq;
    for (unsigned long t = visto + TiempoAceleracionRound3 / 2; t <= visto + TiempoEmbestida + 300; t += 10) {
        c.ejecutar({TipoAccion::AtaqueFrontal, true}, m, t);
    }
    TEST_ASSERT_TRUE(m.izq > antes);
    TEST_ASSERT_EQUAL_INT(VelocidadEmpuje, m.izq);
}

void test_la_linea_cancela_la_rutina(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(3, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::EvadirBordeAmbos}, m, 1010);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    assertNoAvanza(m);
}

// Línea vista estando parado: retrocede recto (o, con SalirGirandoSiNoAvanza,
// gira en el sitio sin retroceder)
void test_linea_sin_venir_avanzando(void) {
    for (int lado = -1; lado <= 1; lado += 2) {
        ControlMovimiento c(CLASICO);
        MotorMock m;
        const TipoAccion borde = lado < 0 ? TipoAccion::EvadirBordeIzq : TipoAccion::EvadirBordeDer;
        // Parado cuando un sensor pisa la línea
        c.ejecutar({borde}, m, 1000);
        if (!SalirGirandoSiNoAvanza) {
            // Retrocede recto mientras ve la línea: nunca gira junto al borde
            for (unsigned long t = 1010; t < 1200; t += 10) {
                c.ejecutar({borde}, m, t);
                TEST_ASSERT_TRUE(m.izq < 0 && m.der < 0 && m.izq == m.der);
            }
            continue;
        }
        assertGiroEvasion(m, -lado);
        for (unsigned long t = 1010; t < 1200; t += 10) {
            c.ejecutar({borde}, m, t);
            assertGiroEvasion(m, -lado);
        }
        // Deja de ver la línea: sigue girando para apartarse y después se para
        c.ejecutar({TipoAccion::Busqueda}, m, 1200);
        assertGiroEvasion(m, -lado);
        const unsigned long separado = 1200 + TiempoSeparacionUnSensor;
        c.ejecutar({TipoAccion::Busqueda}, m, separado);
        assertGiroEvasion(m, -lado);
        c.ejecutar({TipoAccion::Busqueda}, m, separado + TiempoGiroEvasion);
        assertMovimiento(m, 0, 0);
    }
}

void test_sin_interruptores_la_rutina_es_la_del_round_1(void) {
    const int base = RutinasColocacionLibre ? 5 : 1;
    TEST_ASSERT_EQUAL_INT(base, rutinaSegunInterruptores(0));
    TEST_ASSERT_EQUAL_INT(base + 1, rutinaSegunInterruptores(1));
    TEST_ASSERT_EQUAL_INT(base + 2, rutinaSegunInterruptores(2));
    TEST_ASSERT_EQUAL_INT(base + 3, rutinaSegunInterruptores(3));
    TEST_ASSERT_EQUAL_INT(base, rutinaSegunInterruptores(4));  // DIP3 solo elige el lado
}

void test_rutina_frente_reacciona_a_los_laterales(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(3, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    // Rival visto por el lateral izquierdo: termina la rutina y gira hacia él
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000 + TiempoMinimoAvanceInicio);
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1000 + TiempoMinimoAvanceInicio + 2 * ConfirmacionDeteccion);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    TEST_ASSERT_TRUE(m.izq < m.der);
}

// Confirma una decisión durante ConfirmacionDeteccion desde t
static void confirmar(ControlMovimiento& c, MotorMock& m, DecisionMovimiento d, unsigned long t) {
    c.ejecutar(d, m, t);
    c.ejecutar(d, m, t + ConfirmacionDeteccion);
}

void test_contragolpe_espera_y_golpea_al_verlo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(5, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertMovimiento(m, 0, 0);
    if (!ContragolpeAlVerlo) {
        // Solo el central (aún lejos): sigue quieto hasta verlo con dos frontales
        confirmar(c, m, {TipoAccion::AtaqueFrontal, true}, 1100);
        assertMovimiento(m, 0, 0);
        TEST_ASSERT_EQUAL_INT(5, c.rutinaActual());
    }
    confirmar(c, m, {TipoAccion::AjusteIzq, true}, 1200);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    TEST_ASSERT_EQUAL_INT(VelocidadEmpuje, m.der);
}

void test_contragolpe_sin_ver_nada_avanza_despacio(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(5, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxEsperaRival);
    TEST_ASSERT_EQUAL_INT(3, c.rutinaActual());
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxEsperaRival + 200);
    TEST_ASSERT_TRUE(m.izq > 0 && m.izq == m.der && m.izq <= VelocidadAvanceInicio);
}

void test_emboscada_pivota_hacia_el_lateral_y_golpea(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(6, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertMovimiento(m, 0, 0);
    // Pasa por el lateral izquierdo: pivota sobre la rueda izquierda
    confirmar(c, m, {TipoAccion::DefensaIzq}, 1100);
    assertMovimiento(m, 0, VelocidadPivoteEmboscada);
    // De frente: golpe a fondo
    confirmar(c, m, {TipoAccion::AtaqueFrontal, true}, 1200);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    assertMovimiento(m, VelocidadEmpuje, VelocidadEmpuje);
}

void test_de_lado_espera_mientras_solo_lo_ve_el_lateral(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(7, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    confirmar(c, m, {TipoAccion::DefensaDer}, 1100);
    assertMovimiento(m, 0, 0);
    // Sigue en el lateral TiempoLateralCerca: ya está encima, pivota hacia él
    c.ejecutar({TipoAccion::DefensaDer}, m, 1100 + TiempoLateralCerca);
    assertMovimiento(m, VelocidadPivoteDeLado, 0);
}

void test_espera_y_esquiva_se_aparta_si_viene_rapido(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(8, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    // Lo ve el frontal poco después de salir: viene rápido, se aparta hacia
    // DIP3 (derecha)
    confirmar(c, m, {TipoAccion::AtaqueFrontal, true}, 1000 + VentanaRivalRapido / 2);
    assertMovimiento(m, VelocidadEsquivaLibre, 0);
    TEST_ASSERT_EQUAL_INT(8, c.rutinaActual());
    // Acabada la esquiva, sigue el control normal
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + VentanaRivalRapido / 2 + TiempoEsquivaLibre + 10);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
}

void test_espera_y_esquiva_golpea_si_viene_despacio(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(8, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + VentanaRivalRapido + 10);
    // Aparece tarde: espera a tenerlo cerca y golpea
    confirmar(c, m, {TipoAccion::AtaqueFrontal, true}, 1000 + VentanaRivalRapido + 20);
    TEST_ASSERT_EQUAL_INT(8, c.rutinaActual());
    DecisionMovimiento directo = {TipoAccion::AtaqueFrontal, true, false, true};
    confirmar(c, m, directo, 1000 + VentanaRivalRapido + 50);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    assertMovimiento(m, VelocidadEmpuje, VelocidadEmpuje);
}

void test_rutina_libre_gira_hasta_ver_al_rival(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(4, -1);
    // Sin ver nada: gira en el sitio hacia el lado de DIP3
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    assertMovimiento(m, -VelocidadGiroRound4, VelocidadGiroRound4);
    TEST_ASSERT_EQUAL_INT(4, c.rutinaActual());
    // Un lateral lo ve: termina la rutina y gira hacia él
    c.ejecutar({TipoAccion::DefensaDer}, m, 1100);
    c.ejecutar({TipoAccion::DefensaDer}, m, 1100 + 2 * ConfirmacionDeteccion);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
    TEST_ASSERT_TRUE(m.izq > m.der);
}

void test_rutina_libre_termina_al_agotar_su_tiempo(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(4, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoMaxGiroRound4 + 1);
    TEST_ASSERT_EQUAL_INT(0, c.rutinaActual());
}

void test_rutina_lado_elige_el_lado_con_el_sensor_lateral(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    // El DIP3 dice izquierda, pero el lateral derecho ve al rival: pivota a la derecha
    c.iniciarRutina(2, -1);
    c.ejecutar({TipoAccion::DefensaDer}, m, 1000);
    c.ejecutar({TipoAccion::DefensaDer}, m, 1000 + TiempoEsquivaRound2);
    assertGiroRound2(m, 1);
    // Una vez decidido, no cambia de lado aunque luego lo vea el otro lateral
    c.ejecutar({TipoAccion::DefensaIzq}, m, 1010 + TiempoEsquivaRound2);
    assertGiroRound2(m, 1);
}

// El sensor lateral ve al rival al empezar, pero al avanzar en la esquiva lo
// deja atrás y ya no lo ve: aun así gira hacia ese lado, no hacia el de DIP3
void test_rutina_lado_recuerda_el_lado_visto_durante_la_esquiva(void) {
    if (TiempoEsquivaRound2 == 0) {
        return;
    }
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(2, -1);                               // DIP3: izquierda
    c.ejecutar({TipoAccion::DefensaDer}, m, 1000);        // el lateral derecho lo ve
    assertMovimiento(m, VelocidadEsquiva, VelocidadEsquiva);
    c.ejecutar({TipoAccion::Busqueda}, m, 1050);          // al avanzar deja de verlo
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoEsquivaRound2);
    assertGiroRound2(m, 1);                               // gira a la derecha
}

void test_rutina_lado_sin_lateral_usa_el_dip3(void) {
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.iniciarRutina(2, 1);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000);
    c.ejecutar({TipoAccion::Busqueda}, m, 1000 + TiempoEsquivaRound2);
    assertGiroRound2(m, 1);
}

void test_visto_a_45_pivota_sobre_la_rueda_de_ese_lado(void) {
    if (!Corregir45EnPivote || UsarPID) {
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
    const unsigned long visto = 1000 + TiempoEsquivaRound1 + TiempoMinimoGiroEspalda + 2 * TauRuedas;
    // Al girar, el rival pasa primero por el sensor de 45° y luego queda de frente
    c.ejecutar({TipoAccion::CorregirDer}, m, visto - 20);
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
    c.ejecutar({TipoAccion::Busqueda}, m, 5000 + TiempoEsquivaRound1);
    assertGiroRound1(m, 1);
}

// --- PID de ataque ---
static DecisionMovimiento conError(TipoAccion tipo, int error) {
    return {tipo, false, false, false, error};
}

// Parte proporcional: frena la rueda del lado del rival según el error
void test_pid_proporcional(void) {
    // Aísla el PID de la corrección aprendida que se suma a su salida.
    neutralizarRedControl();
    if (!UsarPID || PIDDiferencial) return;
    ControlMovimiento c(CLASICO);
    MotorMock m;
    const int error = -300;   // rival algo a la izquierda
    c.ejecutar(conError(TipoAccion::AjusteIzq, error), m, 1000);
    assertMovimiento(m, (int)(VelocidadAtaque * (1.0f + KpPID * error)), VelocidadAtaque);
    // Error grande: corrección limitada a 1 (rueda parada, pivote)
    c.ejecutar(conError(TipoAccion::CorregirDer, 1000000), m, 1001);
    assertMovimiento(m, VelocidadAtaque, 0);
}

// Parte derivativa: un cambio brusco del error gira más que la parte P sola
void test_pid_derivativo(void) {
    // Aísla el PID de la corrección aprendida que se suma a su salida.
    neutralizarRedControl();
    if (!UsarPID || KdPID <= 0) return;
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar(conError(TipoAccion::AtaqueFrontal, 0), m, 1000);
    assertMovimiento(m, VelocidadAtaque, VelocidadAtaque);
    const int error = 100;
    c.ejecutar(conError(TipoAccion::AjusteDer, error), m, 1000 + PeriodoPID);
    TEST_ASSERT_TRUE(m.der < (int)(m.izq * (1.0f - KpPID * error)));
}

// Tras una pausa sin atacar, el PID empieza de cero: solo la parte P
void test_pid_olvida_tras_una_pausa(void) {
    // Aísla el PID de la corrección aprendida que se suma a su salida.
    neutralizarRedControl();
    if (!UsarPID || PIDDiferencial) return;
    ControlMovimiento c(CLASICO);
    MotorMock m;
    c.ejecutar(conError(TipoAccion::AjusteDer, 100), m, 1000);
    c.ejecutar(conError(TipoAccion::AjusteIzq, -100), m, 1000 + PausaReinicioPID + 1);
    // La rueda derecha va a la velocidad de ataque (que ya ha subido algo)
    TEST_ASSERT_INT_WITHIN(1, (int)(m.der * (1.0f - KpPID * 100)), m.izq);
}

void test_segundo_sensor_amplia_retroceso_sin_reiniciar_tope(void) {
    ManiobraEvasion evasion;
    MandoMotores mando(0);
    MotorMock motor;
    BusquedaRival busqueda;
    TEST_ASSERT_TRUE(evasion.iniciar(1, 90, 100, 1000, true));
    TEST_ASSERT_FALSE(evasion.iniciar(1, 200, 100, 1010, false));
    evasion.continuar({TipoAccion::Busqueda}, mando, motor, 1010, busqueda);
    evasion.continuar({TipoAccion::Busqueda}, mando, motor, 1110, busqueda);
    assertMovimiento(motor, -VelocidadRetroceso, -VelocidadRetroceso);
    evasion.continuar({TipoAccion::Busqueda}, mando, motor, 1200, busqueda);
    assertMovimiento(motor, 0, 0);
    ManiobraEvasion agotada;
    agotada.iniciar(1, 90, 100, 1000, true);
    agotada.iniciar(1, 200, 100, 1010, false);
    agotada.continuar({TipoAccion::EvadirBordeAmbos}, mando, motor,
                     1000 + TiempoMaximoRecuperacionBorde, busqueda);
    assertMovimiento(motor, 0, 0);
}

// La corrección aprendida sigue sumándose al mando completo de ataque.
void test_red_control_se_suma_al_ataque(void) {
    if (!UsarRedControl) return;
    neutralizarRedControl();
    ControlMovimiento base(CLASICO);
    MotorMock sinCorreccion;
    base.ejecutar(conError(TipoAccion::AtaqueFrontal, 0), sinCorreccion, 1000);

    const int n = redcontrol::numParametros();
    const int ocultas = (n - 2) / (redcontrol::NumEntradasRed + 1 + 2);
    // Sesgo de avance: -64 / 16 = -4 PWM, sin depender de pesos entrenados.
    redcontrol::Parametros[n - ocultas - 2] = -64;
    ControlMovimiento corregido(CLASICO);
    MotorMock conCorreccion;
    corregido.ejecutar(conError(TipoAccion::AtaqueFrontal, 0), conCorreccion, 1000);
    assertMovimiento(conCorreccion, sinCorreccion.izq - 4, sinCorreccion.der - 4);
}

// Red de control: con todo a cero no corrige; la salida es la suma / 16, limitada a ±255
void test_red_control_escala_y_limita(void) {
    const int n = redcontrol::numParametros();
    memset(redcontrol::Parametros, 0, n);
    int8_t x[redcontrol::NumEntradasRed] = {8, 0, 0, 0, 0, 0, 0};
    int avance = 1, giro = 1;
    redcontrol::corregir(x, avance, giro);
    TEST_ASSERT_EQUAL_INT(0, avance);
    TEST_ASSERT_EQUAL_INT(0, giro);
    // Primera neurona oculta = 2 × error (16); el avance la mira con peso 4: 64 / 16 = 4
    const int e = redcontrol::NumEntradasRed;
    const int ocultas = (n - 2) / (e + 1 + 2);
    redcontrol::Parametros[0] = 2;
    redcontrol::Parametros[ocultas * (e + 1)] = 4;
    redcontrol::corregir(x, avance, giro);
    TEST_ASSERT_EQUAL_INT(4, avance);
    TEST_ASSERT_EQUAL_INT(0, giro);
    // Muy grande: se limita
    redcontrol::Parametros[0] = 127;
    redcontrol::Parametros[ocultas * (e + 1)] = 127;
    redcontrol::corregir(x, avance, giro);
    TEST_ASSERT_EQUAL_INT(255, avance);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_busqueda_tras_un_sensor_sigue_hacia_el_lado_del_escape);
    RUN_TEST(test_segundo_sensor_cambia_a_recto_sin_reiniciar_limite);
    RUN_TEST(test_arranque_no_dispara_aunque_tres_frontales_vean_enemigo);
    RUN_TEST(test_tres_frontales_cancelan_giro_y_pausa_pero_no_evasion);
    RUN_TEST(test_ver_al_rival_corta_la_evasion);
    RUN_TEST(test_un_sensor_retrocede_recto_y_gira_tras_separarse);
    RUN_TEST(test_ambos_retrocede_sin_reinicios_y_blanco_interrumpe_giro);
    RUN_TEST(test_busqueda_default_en_curvas_sin_paradas_y_borde_la_interrumpe);
    RUN_TEST(test_linea_persistente_no_se_queda_parado_ni_avanza);
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
    RUN_TEST(test_rutina_frente_respeta_potencia_configurada);
    RUN_TEST(test_la_linea_cancela_la_rutina);
    RUN_TEST(test_linea_sin_venir_avanzando);
    RUN_TEST(test_sin_interruptores_la_rutina_es_la_del_round_1);
    RUN_TEST(test_giro_lateral_ignora_un_frontal_antes_del_giro_minimo);
    RUN_TEST(test_rutina_frente_reacciona_a_los_laterales);
    RUN_TEST(test_contragolpe_espera_y_golpea_al_verlo);
    RUN_TEST(test_contragolpe_sin_ver_nada_avanza_despacio);
    RUN_TEST(test_emboscada_pivota_hacia_el_lateral_y_golpea);
    RUN_TEST(test_de_lado_espera_mientras_solo_lo_ve_el_lateral);
    RUN_TEST(test_espera_y_esquiva_se_aparta_si_viene_rapido);
    RUN_TEST(test_espera_y_esquiva_golpea_si_viene_despacio);
    RUN_TEST(test_rutina_libre_gira_hasta_ver_al_rival);
    RUN_TEST(test_rutina_libre_termina_al_agotar_su_tiempo);
    RUN_TEST(test_rutina_lado_elige_el_lado_con_el_sensor_lateral);
    RUN_TEST(test_rutina_lado_sin_lateral_usa_el_dip3);
    RUN_TEST(test_rutina_lado_recuerda_el_lado_visto_durante_la_esquiva);
    RUN_TEST(test_visto_a_45_pivota_sobre_la_rueda_de_ese_lado);
    RUN_TEST(test_round_1_mantiene_su_velocidad_tras_la_embestida);
    RUN_TEST(test_rutina_frente_espera_quieto_tras_avanzar);
    RUN_TEST(test_reiniciar_vuelve_al_estado_inicial);
    RUN_TEST(test_pid_proporcional);
    RUN_TEST(test_pid_derivativo);
    RUN_TEST(test_pid_olvida_tras_una_pausa);
    RUN_TEST(test_red_control_escala_y_limita);
    RUN_TEST(test_red_control_se_suma_al_ataque);
    RUN_TEST(test_segundo_sensor_amplia_retroceso_sin_reiniciar_tope);
    return UNITY_END();
}
