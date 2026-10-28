import unittest

from simulacion.python import tablero


def bateria(v, c, p, n=100):
    t = {"combates": n, "victorias": v, "caidas": c, "caidas_propias": p}
    return {"total": t, "por_modo": {"1": t}, "por_rival": {"embiste": t}}


class Tablero(unittest.TestCase):
    def test_elige_el_de_menor_error_y_penaliza_caidas_propias(self):
        cands = [("base", bateria(50, 30, 0)), ("gana_mas_pero_se_cae_solo", bateria(60, 30, 5)),
                 ("mejor", bateria(55, 25, 0))]
        resumen, rivales = tablero.tabla_candidatos(cands)
        self.assertIn("Mejor: <b>mejor</b>", resumen)
        self.assertIn("mejor gana/cae", rivales)


if __name__ == "__main__":
    unittest.main()
