#!/usr/bin/env python3
"""Verifica las fuentes protegidas y, opcionalmente, dos baterías completas."""
import argparse
import json
from pathlib import Path
import subprocess

RAIZ = Path(__file__).resolve().parent.parent

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--referencia', default='c8136d6')
    ap.add_argument('--antes', type=Path)
    ap.add_argument('--despues', type=Path)
    a = ap.parse_args()
    archivos = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', a.referencia,
                                         'src', 'include'], cwd=RAIZ, text=True).splitlines()
    for nombre in archivos:
        if not nombre.endswith(('.cpp', '.h')):
            continue
        original = subprocess.check_output(['git', 'show', f'{a.referencia}:{nombre}'], cwd=RAIZ)
        actual = (RAIZ / 'firmware' / nombre).read_bytes()
        if original != actual:
            raise SystemExit(f'ERROR: fuente protegida modificada: {nombre}')
    print('Firmware, parámetros y tablas: idénticos byte a byte a la referencia.')
    if bool(a.antes) != bool(a.despues):
        ap.error('--antes y --despues deben usarse juntos')
    if a.antes and a.despues:
        antes, despues = (json.loads(p.read_text()) for p in (a.antes, a.despues))
        if antes != despues:
            raise SystemExit('ERROR: difieren las baterías (incluidas casillas y totales).')
        print(f"Baterías idénticas: {len(antes['casillas'])} casillas; {antes['total']}.")

if __name__ == '__main__':
    main()
