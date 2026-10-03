#include <unity.h>
#include "ControladorRobot.H"
#include "Arduino.h"

class MotorDummy : public IMotor {
public:
    void detener() override {}
    void mover(int, int) override {}
};

class PercepcionMock : public IPercepcion {
public:
    mutable int calls = 0;
    LecturasSensores next{};

    LecturasSensores leer() const override {
        calls++;
        return next;
    }
};

class EstrategiaMock : public IEstrategiaCombate {
public:
    mutable int calls = 0;
    mutable LecturasSensores recibido{};
    DecisionMovimiento salida{TipoAccion::Busqueda};

    DecisionMovimiento decidir(const LecturasSensores& lecturas) const override {
        calls++;
        recibido = lecturas;
        return salida;
    }
};

class ControlMock : public IControlMovimiento {
public:
    int calls = 0;
    TipoAccion ultima = TipoAccion::Busqueda;
    unsigned long ultimoTiempo = 0;

    void ejecutar(const DecisionMovimiento& decision, IMotor&, unsigned long ahora) override {
        calls++;
        ultima = decision.tipo;
        ultimoTiempo = ahora;
    }
};

void test_orquesta_leer_decidir_ejecutar(void) {
    PercepcionMock p;
    EstrategiaMock e;
    ControlMock c;
    MotorDummy m;

    p.next = {false, true, false, false, false, false, false};
    e.salida = {TipoAccion::EvadirBordeDer};
    g_millis = 1234;

    ControladorRobot robot(p, e, c, m);
    robot.actualizar();

    TEST_ASSERT_EQUAL_INT(1, p.calls);
    TEST_ASSERT_EQUAL_INT(1, e.calls);
    TEST_ASSERT_EQUAL_INT(1, c.calls);
    TEST_ASSERT_EQUAL_INT((int)TipoAccion::EvadirBordeDer, (int)c.ultima);
    TEST_ASSERT_EQUAL_UINT32(1234, c.ultimoTiempo);

    TEST_ASSERT_EQUAL_INT((int)p.next.lineaDer, (int)e.recibido.lineaDer);
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_orquesta_leer_decidir_ejecutar);
    return UNITY_END();
}
