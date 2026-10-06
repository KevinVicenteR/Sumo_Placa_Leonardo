#include <unity.h>
#include "movimiento/EstimadorBorde.h"
#include "Parametros.h"

void test_al_arrancar_no_se_fia_de_la_prediccion(void) {
    EstimadorBorde e;
    TEST_ASSERT_FALSE(e.confiable());
    // Aun sin saber el rumbo, cerca del centro hay sitio para avanzar algo
    TEST_ASSERT_TRUE(e.distanciaLibre() > 0.05f);
}

void test_al_ver_la_linea_de_frente_el_borde_esta_encima(void) {
    EstimadorBorde e;
    e.lineaVista(0, true);
    TEST_ASSERT_TRUE(e.confiable());
    TEST_ASSERT_TRUE(e.distanciaLibre() < 0.01f);
    TEST_ASSERT_EQUAL_INT(VelocidadMinimaEstimador, e.limiteAvance());
}

void test_tras_dar_media_vuelta_tiene_el_dohyo_por_delante(void) {
    EstimadorBorde e;
    e.lineaVista(0, true);
    // Gira en el sitio hasta mirar hacia el centro (aprox. media vuelta)
    float girado = 0;
    while (girado < 3.14159f) {
        e.predecir(150, -150, 0.001f, 0);
        girado += 2 * EstimadorBorde::velocidadRegimen(150) / TrochaRuedas * 0.001f;
    }
    for (int i = 0; i < 100; i++) e.predecir(0, 0, 0.001f, 0);
    // Con un giro tan largo el rumbo ya es incierto, pero la distancia al centro no
    TEST_ASSERT_TRUE(e.incertidumbrePosicion() < IncertPosicionConfiable);
}

void test_la_incertidumbre_crece_al_moverse_y_con_empujones(void) {
    EstimadorBorde e;
    e.lineaVista(0, true);
    const float antes = e.incertidumbrePosicion();
    for (int i = 0; i < 200; i++) e.predecir(-200, -200, 0.001f, 0);
    const float tras = e.incertidumbrePosicion();
    TEST_ASSERT_TRUE(tras > antes);
    for (int i = 0; i < 200; i++) e.predecir(0, 0, 0.001f, EmpujonContacto);
    TEST_ASSERT_TRUE(e.incertidumbrePosicion() > tras);
}

void test_retroceder_desde_la_linea_aleja_del_borde(void) {
    EstimadorBorde e;
    e.lineaVista(0, true);
    for (int i = 0; i < 150; i++) e.predecir(-200, -200, 0.001f, 0);
    const float r = e.x() * e.x() + e.y() * e.y();
    const float rLinea = RadioDohyo - AnchoLineaBorde - DistanciaSensorPiso;
    TEST_ASSERT_TRUE(r < rLinea * rLinea);
}

void test_modelo_de_rueda_con_zona_muerta(void) {
    TEST_ASSERT_EQUAL_FLOAT(0, EstimadorBorde::velocidadRegimen(ZonaMuertaPwm));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, VelocidadRuedaMaxima, EstimadorBorde::velocidadRegimen(255));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -VelocidadRuedaMaxima, EstimadorBorde::velocidadRegimen(-255));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_al_arrancar_no_se_fia_de_la_prediccion);
    RUN_TEST(test_al_ver_la_linea_de_frente_el_borde_esta_encima);
    RUN_TEST(test_tras_dar_media_vuelta_tiene_el_dohyo_por_delante);
    RUN_TEST(test_la_incertidumbre_crece_al_moverse_y_con_empujones);
    RUN_TEST(test_retroceder_desde_la_linea_aleja_del_borde);
    RUN_TEST(test_modelo_de_rueda_con_zona_muerta);
    return UNITY_END();
}
