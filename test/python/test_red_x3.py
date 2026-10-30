"""Equivalencia exacta entre los pesos planos originales del X3 y su conversión jerárquica."""
import itertools
import re
import subprocess
import unittest
from simulacion.python import redes as R


class RedX3(unittest.TestCase):
    def test_todas_las_entradas_conservan_sumas_y_decisiones(self):
        texto = subprocess.check_output([
            'git', 'show', 'X3_pesos_neuronales_rumania:include/estrategia/TablaRedLogica.h'
        ], cwd=R.RAIZ, text=True)
        cuerpo = texto.split('= {', 1)[1].split('};', 1)[0]
        plana = [int(v) for v in re.findall(r'-?\d+', re.sub(r'//[^\n]*', '', cuerpo))]
        # El firmware ya lleva la red reentrenada: se comprueba la conversión, no la tabla
        jerarquica = R.a_jerarquica(plana)
        for x in itertools.product((0, 1), repeat=12):
            sumas, subred = R.adelante_logica(jerarquica, x)
            self.assertEqual(R.adelante(plana, R.LOGICA, x), sumas)
            self.assertEqual(0, subred)
            for minima in (0, 100, 10000):
                self.assertEqual(R.accion(plana, x, minima), R.accion(jerarquica, x, minima))
