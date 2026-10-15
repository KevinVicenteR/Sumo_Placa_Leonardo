#!/usr/bin/env python3
"""Las dos redes del robot: tamaños, pasada hacia delante y tablas.

Red de lógica (qué hacer): 12 entradas 0/1 -> 14 ReLU -> 10 acciones (gana la
mayor; si no llega a la suma mínima, Busqueda). Red de control (cómo mover las
ruedas): 7 entradas -> 6 ReLU -> 2 correcciones (avance y giro, la suma / 16)
que se suman al movimiento de siempre. Todo en enteros int8, igual que RedLogica.cpp y
RedControl.cpp.

Los parámetros de cada red van seguidos en un vector, en el mismo orden que el
.h, que lee el robot y que carga el simulador con --red: una fila por neurona
oculta (sus pesos y al final su sesgo) y después una fila por salida.
"""
import re
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent

ENTRADAS_LOGICA = ["Fr", "45I", "45D", "LatI", "LatD", "Lado", "Borde", "Perd", "Emp",
                   "BordeDelante", "Esquivando", "Rutina"]
ACCIONES = ["AtaqueFrontal", "AjusteIzq", "AjusteDer", "CorregirIzq", "CorregirDer",
            "DefensaIzq", "DefensaDer", "Busqueda", "EsquivarIzq", "EsquivarDer"]
ENTRADAS_CONTROL = ["Error", "Cambio", "Empujando", "TiempoAtaque", "BordeDelante",
                    "Velocidad", "Esquiva"]
SALIDAS_CONTROL = ["Avance", "Giro"]

LOGICA = (len(ENTRADAS_LOGICA), 14, len(ACCIONES))
CONTROL = (len(ENTRADAS_CONTROL), 6, len(SALIDAS_CONTROL))
NEURONA_BUSQUEDA = ACCIONES.index("Busqueda")

TABLA_LOGICA = RAIZ / "include" / "estrategia" / "TablaRedLogica.h"
TABLA_CONTROL = RAIZ / "include" / "movimiento" / "TablaRedControl.h"


def num_parametros(forma):
    e, h, s = forma
    return h * e + h + s * h + s


def separar(p, forma):
    e, h, s = forma
    w1 = [p[j * (e + 1):j * (e + 1) + e] for j in range(h)]
    b1 = [p[j * (e + 1) + e] for j in range(h)]
    o = h * (e + 1)
    w2 = [p[o + k * (h + 1):o + k * (h + 1) + h] for k in range(s)]
    b2 = [p[o + k * (h + 1) + h] for k in range(s)]
    return w1, b1, w2, b2


def unir(w1, b1, w2, b2):
    return [x for f, b in zip(w1, b1) for x in f + [b]] + [x for f, b in zip(w2, b2) for x in f + [b]]


def adelante(p, forma, x):
    """Sumas de salida (enteros), como en el robot (la de control, / 16)."""
    w1, b1, w2, b2 = separar(p, forma)
    h = [max(0, b + sum(w * xi for w, xi in zip(fila, x))) for fila, b in zip(w1, b1)]
    sumas = [b + sum(w * hj for w, hj in zip(fila, h)) for fila, b in zip(w2, b2)]
    return sumas if forma == LOGICA else [max(-255, min(255, v >> 4)) for v in sumas]


def accion(p, x, minima=0):
    sumas = adelante(p, LOGICA, x)
    mejor = max(range(len(sumas)), key=lambda k: (sumas[k], -k))
    return mejor if sumas[mejor] >= minima else NEURONA_BUSQUEDA


def control_inicial():
    """Todo a cero: la red de control no corrige nada (movimiento de siempre)."""
    return [0] * num_parametros(CONTROL)


def escribir_tabla(ruta, guarda, espacio, nombre, forma, p, entradas, salidas):
    w1, b1, w2, b2 = separar(p, forma)
    filas = []
    for fila, b in zip(w1, b1):
        filas.append("    " + ", ".join(f"{v:4d}" for v in fila) + f",  {b:4d},")
    filas.append("    // salidas")
    for nom, fila, b in zip(salidas, w2, b2):
        filas.append("    " + ", ".join(f"{v:4d}" for v in fila) + f",  {b:4d},   // {nom}")
    texto = f"""// {ruta.name} — Pesos de la {nombre} (los escribe simulacion/entrenar_redes.py)
// {forma[0]} entradas -> {forma[1]} neuronas ReLU -> {forma[2]} salidas, enteros int8.
// Cada fila: los pesos de una neurona y, al final, su sesgo. Primero las
// {forma[1]} neuronas ocultas (entradas: {", ".join(entradas)}) y después las salidas.
#ifndef {guarda}
#define {guarda}

#include "estrategia/TablaRed.h"

namespace {espacio} {{

constexpr int NumEntradas = {forma[0]};
constexpr int NumOcultas = {forma[1]};
constexpr int NumSalidas = {forma[2]};
constexpr int NumParametros = NumOcultas * (NumEntradas + 1) + NumSalidas * (NumOcultas + 1);

TABLA_RED(Parametros, NumParametros) = {{
""" + "\n".join(filas) + f"""
}};

}}

#endif
"""
    ruta.write_text(texto)


def leer_tabla(ruta):
    texto = ruta.read_text().split("= {", 1)[1].split("};", 1)[0]
    texto = re.sub(r"//[^\n]*", "", texto)
    return [int(x) for x in re.findall(r"-?\d+", texto)]


def escribir_logica(p):
    escribir_tabla(TABLA_LOGICA, "TABLA_RED_LOGICA_H", "redlogica", "red de lógica", LOGICA, p,
                   ENTRADAS_LOGICA, ACCIONES)


def escribir_control(p):
    escribir_tabla(TABLA_CONTROL, "TABLA_RED_CONTROL_H", "redcontrol", "red de control", CONTROL, p,
                   ENTRADAS_CONTROL, SALIDAS_CONTROL)


def guardar_pesos(ruta, logica, control):
    """Archivo para --red del simulador: los parámetros de las dos redes."""
    Path(ruta).write_text(" ".join(map(str, list(logica) + list(control))) + "\n")


def propuesta_esquiva(logica, fuerza=64):
    """Hipótesis para evaluar, nunca publicar sin validación.

    Las salidas originales de esquiva tienen un sesgo cercano a -1600, por
    lo que mutaciones pequeñas casi nunca llegan a activarlas. Arranca una
    ruta explícita frontal persistente -> arco lateral, con el lado recordado.
    """
    w1, b1, w2, b2 = separar(logica, LOGICA)
    e, h, _ = LOGICA
    j = h - 1
    w1[j] = [0] * e
    w1[j][0] = w1[j][8] = 16
    b1[j] = -16
    for fila in w2:
        fila[j] = 0
    for k, lado, sesgo in [(8, -16, -40), (9, 16, -56)]:
        w2[k] = [0] * h
        w2[k][j] = fuerza
        w2[k][5] = lado
        w2[k][12] = sesgo
        b2[k] = 0
    return unir(w1, b1, w2, b2)
