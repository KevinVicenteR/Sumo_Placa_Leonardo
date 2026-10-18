"""Regresiones de selección: una victoria extra no justifica más caídas."""
import unittest
from simulacion.python import redes as R
from simulacion.python.entrenar_redes import mejora_segura, prioridad_validacion, puntuar, elegir_candidato


class SeleccionSegura(unittest.TestCase):
    def setUp(self):
        self.obj = (100, 100, 0, 0)
        self.total = (-700, 40, 37, 1)

    def test_mas_caidas_no_se_publican_aunque_gane_mas(self):
        self.assertFalse(mejora_segura(self.obj, (-710, 50, 38, 1), self.obj, self.total))

    def test_menos_caidas_y_victorias_conservadas_se_aceptan(self):
        self.assertTrue(mejora_segura(self.obj, (-660, 40, 35, 1), self.obj, self.total))

    def test_no_se_acepta_dejar_de_competir_para_evitar_caidas(self):
        self.assertFalse(mejora_segura(self.obj, (0, 0, 0, 0), self.obj, self.total))

    def test_no_se_empeora_seguridad_del_objetivo(self):
        self.assertFalse(mejora_segura((80, 100, 1, 0), (-660, 40, 35, 1), self.obj, self.total))

    def test_una_validacion_vacia_no_se_presenta_como_cero_caidas(self):
        with self.assertRaises(ValueError):
            puntuar([])

    def test_menos_caidas_totales_no_ocultan_mas_caidas_propias(self):
        self.assertFalse(mejora_segura(self.obj, (-660, 40, 35, 2), self.obj, self.total))

    def test_un_candidato_invalido_no_oculta_otro_que_si_mejora(self):
        deja_de_ganar = ([1], self.obj, (0, 10, 0, 0))
        mejora_aceptable = ([2], self.obj, (-660, 40, 35, 0.5))
        elegida = elegir_candidato([deja_de_ganar, mejora_aceptable], self.obj, self.total,
                                  self.obj, self.total)
        self.assertEqual(elegida, mejora_aceptable)

    def test_propuesta_activa_esquiva_solo_tras_frontal_persistente(self):
        # Una hipótesis se prueba con una red controlada: los pesos entrenados
        # actuales pueden favorecer otras acciones y no garantizan esta salida.
        plana = [0] * R.num_parametros(R.LOGICA)
        p = R.propuesta_esquiva(R.a_jerarquica(plana), 96)
        x = [1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0]
        self.assertIn(R.accion(p, x), (8, 9))
        x[8] = 0
        self.assertNotIn(R.accion(p, x), (8, 9))

    def test_rapidez_solo_cuenta_victorias_y_no_compensa_caidas(self):
        rapida = puntuar([(10, 0, 10, 0, 10)])
        lenta = puntuar([(10, 0, 10, 0, 200)])
        derrota = puntuar([(0, 10, 10, 0, 0)])
        self.assertGreater(rapida[0], lenta[0])
        self.assertLess(derrota[0], lenta[0])
        self.assertEqual(rapida[4], 1)

    def test_identico_no_cuenta_como_mejora(self):
        self.assertFalse(mejora_segura(self.obj, self.total, self.obj, self.total))

    def test_caidas_tienen_prioridad_y_penalizacion(self):
        self.assertGreater(prioridad_validacion(self.obj, (0, 40, 0, 0)),
                           prioridad_validacion(self.obj, (0, 100, 1, 0)))
        self.assertEqual(puntuar([(10, 1, 10)]), (-100, 100, 10, 0, 30))


if __name__ == "__main__":
    unittest.main()
