#include <unity.h>
#include "hardware/ComandosControlIR.h"

namespace {
constexpr CodigoControlIR start{1, 0x10, 0x20, 0};
constexpr CodigoControlIR stop{1, 0x10, 0x21, 0};
constexpr CodigoControlIR reset{2, 0x10, 0x20, 0};
}

void test_identifica_los_tres_comandos(void) {
    TEST_ASSERT_EQUAL(static_cast<int>(AccionControlIR::Iniciar),
                      static_cast<int>(buscarAccionIR(start, start, stop, reset)));
    TEST_ASSERT_EQUAL(static_cast<int>(AccionControlIR::Detener),
                      static_cast<int>(buscarAccionIR(stop, start, stop, reset)));
    TEST_ASSERT_EQUAL(static_cast<int>(AccionControlIR::Reiniciar),
                      static_cast<int>(buscarAccionIR(reset, start, stop, reset)));
}

void test_no_confunde_codigos_de_otro_protocolo_o_comando(void) {
    constexpr CodigoControlIR distinto{2, 0x10, 0x21, 0};
    TEST_ASSERT_EQUAL(static_cast<int>(AccionControlIR::Ninguna),
                      static_cast<int>(buscarAccionIR(distinto, start, stop, reset)));
}

void test_hash_distingue_botones_de_protocolos_no_identificados(void) {
    constexpr CodigoControlIR botonHashStart{0xFF, 0, 0, 0x12345678};
    constexpr CodigoControlIR botonHashStop{0xFF, 0, 0, 0x87654321};
    TEST_ASSERT_EQUAL(static_cast<int>(AccionControlIR::Detener),
                      static_cast<int>(buscarAccionIR(
                          botonHashStop, botonHashStart, botonHashStop, reset)));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_identifica_los_tres_comandos);
    RUN_TEST(test_no_confunde_codigos_de_otro_protocolo_o_comando);
    RUN_TEST(test_hash_distingue_botones_de_protocolos_no_identificados);
    return UNITY_END();
}
