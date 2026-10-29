#!/usr/bin/env python3
"""Compara órdenes de los arranques 1–3 con un ref Git (HEAD por defecto).

python3 -m simulacion.python.diagnostico.verificar_arranques --referencia HEAD --salida informe.json
No usa el robot ni escribe en la referencia. Compara 30 escenarios de sensores,
ambos lados, cinco patrones por modo y dos binarios independientes.
"""
import argparse, collections, csv, io, json, subprocess, tempfile
from pathlib import Path
RAIZ=Path(__file__).resolve().parents[3]

def compilar(raiz, carpeta, nombre):
    firmware = raiz/'firmware' if (raiz/'firmware').exists() else raiz
    fuentes=sorted((firmware/'src/estrategia').glob('*.cpp'))+sorted((firmware/'src/movimiento').glob('*.cpp'))
    fuentes += [raiz/'test/support/arduino_stubs.cpp']
    flags=['-DPORT_MODO4'] if (firmware/'include/estrategia/EstrategiaCompetencia.h').exists() else []
    exe=carpeta/nombre
    subprocess.run(['c++','-std=c++17','-O2',*flags,f'-I{firmware}/include',f'-I{raiz}/test/support',
                    str(RAIZ/'test/simulacion/verificar_arranques.cpp'),*map(str,fuentes),'-o',str(exe)],check=True)
    datos=subprocess.check_output([str(exe)],text=True)
    inicios=[];terminados=set()
    for fila in csv.reader(io.StringIO(datos)):
        clave=tuple(fila[:3])
        if clave in terminados: continue
        inicios.append(fila)
        if fila[7]=='0': terminados.add(clave)
    return inicios

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--referencia',default='HEAD');ap.add_argument('--salida',type=Path)
    args=ap.parse_args()
    sha=subprocess.check_output(['git','rev-parse',args.referencia],cwd=RAIZ,text=True).strip()
    with tempfile.TemporaryDirectory() as tmp:
        carpeta=Path(tmp);base=carpeta/'base';base.mkdir()
        archivo=subprocess.check_output(['git','archive',sha,*( ['firmware','test/support'] if subprocess.call(['git','cat-file','-e',sha+':firmware'],cwd=RAIZ,stderr=subprocess.DEVNULL)==0 else ['include','src','test/support'])],cwd=RAIZ)
        subprocess.run(['tar','-x','-C',str(base)],input=archivo,check=True)
        antes=compilar(base,carpeta,'antes');despues=compilar(RAIZ,carpeta,'despues')
    if antes != despues:
        raise SystemExit('ERROR: cambian órdenes durante un arranque protegido')
    res={'referencia':sha,'identicos':True,'escenarios':30,'ciclos_inicio_comparados':len(antes),
         'ciclos_generados_por_version':150000,'por_modo':dict(collections.Counter(f[0] for f in antes))}
    texto=json.dumps(res,indent=2)
    if args.salida: args.salida.write_text(texto+'\n')
    print(texto)

if __name__=='__main__': main()
