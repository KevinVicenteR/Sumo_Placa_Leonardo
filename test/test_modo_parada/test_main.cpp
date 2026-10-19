#include <unity.h>
#include "Arduino.h"
#define private public
#include "modos/ModoCombate.h"
#undef private
#include "Pines.h"
#include "Parametros.h"
// Dobles del hardware: se cuenta cualquier orden que intente mover motores.
static int movimientos = 0;
void DriverMotores::mover(int, int) { ++movimientos; }
void DriverMotores::detener() { deshabilitar(); }
void DriverMotores::deshabilitar() {}
void PlacaXMotion::inicializarPines() const {}
int PlacaXMotion::leerInterruptores() const { return 0; }
#include "../../src/modos/ModoCombate.cpp"

class EstrategiaPrueba : public IEstrategiaCombate {
public:
    int llamadas = 0;
    DecisionMovimiento decidir(const LecturasSensores&) override {
        ++llamadas;
        return DecisionMovimiento{TipoAccion::Busqueda};
    }
};
struct Banco {
    PlacaXMotion placa;
    DriverMotores motores;
    Percepcion percepcion;
    ControlMovimiento control;
    EstrategiaPrueba estrategia;
    ControladorRobot robot{percepcion, estrategia, control, motores};
    ModoCombate modo{placa, motores, percepcion, control, robot};
    Banco() { g_millis = 0; movimientos = 0; modo.iniciar(); }
    void senal(bool run, unsigned long t) {
        g_millis = t;
        g_digitalValues[(uint8_t)PIN_MODULO_ARRANQUE] =
            run == ModuloArranqueActivoAlto ? HIGH : LOW;
        modo.actualizar();
    }
    void start() {
        senal(false, 0);
        senal(false, StopMinimoAntesDeStart);
        senal(true, StopMinimoAntesDeStart + 1);
        senal(true, StopMinimoAntesDeStart + 1 + FiltroModuloArranqueMs);
    }
};
void test_run_sin_stop_previo_no_arranca() {
    Banco b;
    b.senal(true, 0);
    b.senal(true, 1000);
    TEST_ASSERT_FALSE(b.modo.combateEmpezado);
    b.senal(false, 1001);
    b.senal(true, 1100);
    b.senal(true, 2000);
    TEST_ASSERT_FALSE(b.modo.combateEmpezado);
    TEST_ASSERT_EQUAL(0, movimientos);
    TEST_ASSERT_EQUAL(0, b.estrategia.llamadas);
}
void test_stop_confirmado_reinicia_sin_reanudar() {
    Banco b;
    b.start();
    TEST_ASSERT_TRUE(b.modo.combateEmpezado);
    const unsigned long t = StopMinimoAntesDeStart + 1 + FiltroModuloArranqueMs;
    const int llamadas = b.estrategia.llamadas;
    const int ordenes = movimientos;
    b.senal(false, t + 1);
    b.senal(false, t + 1 + FiltroParadaModuloMs);
    TEST_ASSERT_FALSE(b.modo.combateEmpezado);
    TEST_ASSERT_FALSE(b.modo.enCombate);
    b.senal(true, t + 1 + FiltroParadaModuloMs + 1);
    b.senal(true, t + 1 + FiltroParadaModuloMs + 1000);
    TEST_ASSERT_FALSE(b.modo.combateEmpezado);
    TEST_ASSERT_EQUAL(llamadas, b.estrategia.llamadas);
    TEST_ASSERT_EQUAL(ordenes, movimientos);
    // Un START nuevo requiere su propio STOP estable.
    b.senal(false, 3000);
    b.senal(false, 3300);
    b.senal(true, 3301);
    b.senal(true, 3301 + FiltroModuloArranqueMs);
    TEST_ASSERT_TRUE(b.modo.combateEmpezado);
}
int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_run_sin_stop_previo_no_arranca);
    RUN_TEST(test_stop_confirmado_reinicia_sin_reanudar);
    return UNITY_END();
}
