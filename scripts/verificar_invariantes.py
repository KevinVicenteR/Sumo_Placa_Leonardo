#!/usr/bin/env python3
"""Comprueba que un cambio no altera el robot.

  --referencia COMMIT       el firmware es idéntico byte a byte al de ese commit
  --antes A --despues B     dos baterías (JSON de bateria.py) dan lo mismo por casilla

Sirve para refactorizaciones: no se ejecuta en la CI porque la estrategia
cambia a propósito.
"""
import argparse
import json
from pathlib import Path
import subprocess

RAIZ = Path(__file__).resolve().parent.parent

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--referencia', help='commit cuyo firmware debe ser idéntico')
    ap.add_argument('--antes', type=Path)
    ap.add_argument('--despues', type=Path)
    a = ap.parse_args()
    if a.referencia:
        archivos = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', a.referencia,
                                             'firmware'], cwd=RAIZ, text=True).splitlines()
        for nombre in archivos:
            if not nombre.endswith(('.cpp', '.h')):
                continue
            original = subprocess.check_output(['git', 'show', f'{a.referencia}:{nombre}'], cwd=RAIZ)
            ruta = RAIZ / nombre
            if not ruta.exists() or original != ruta.read_bytes():
                raise SystemExit(f'ERROR: fuente modificada respecto a {a.referencia}: {nombre}')
        print(f'Firmware idéntico byte a byte a {a.referencia}.')
    if bool(a.antes) != bool(a.despues):
        ap.error('--antes y --despues deben usarse juntos')
    if a.antes and a.despues:
        antes, despues = (json.loads(p.read_text()) for p in (a.antes, a.despues))
        if antes != despues:
            raise SystemExit('ERROR: difieren las baterías (incluidas casillas y totales).')
        print(f"Baterías idénticas: {len(antes['casillas'])} casillas; {antes['total']}.")

if __name__ == '__main__':
    main()
