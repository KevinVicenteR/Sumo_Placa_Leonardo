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
# Red de lógica jerárquica: la capa oculta (red principal) alimenta un selector
# que elige cuál de las SUBREDES (cabezas de salida especializadas) decide.
# Orden en la tabla: capa oculta, subred 0, selector, subredes 1..SUBREDES-1
# (las dos primeras partes son exactamente la red plana de antes).
SUBREDES = 3

TABLA_LOGICA = RAIZ / "include" / "estrategia" / "TablaRedLogica.h"
TABLA_CONTROL = RAIZ / "include" / "movimiento" / "TablaRedControl.h"


def num_parametros(forma):
    e, h, s = forma
    return h * e + h + s * h + s


def num_parametros_logica():
    e, h, s = LOGICA
    return num_parametros(LOGICA) + SUBREDES * (h + 1) + (SUBREDES - 1) * s * (h + 1)


def partes_logica(p):
    """(capa oculta, selector, [subredes]) de la red jerárquica; cada parte como
    (pesos por fila, sesgos)."""
    e, h, s = LOGICA
    n = num_parametros(LOGICA)
    w1, b1, w2, b2 = separar(p[:n], LOGICA)
    filas = lambda o, k: [p[o + r * (h + 1):o + (r + 1) * (h + 1)] for r in range(k)]
    sel = filas(n, SUBREDES)
    o = n + SUBREDES * (h + 1)
    subredes = [(w2, b2)]
    for _ in range(SUBREDES - 1):
        f = filas(o, s)
        subredes.append(([x[:h] for x in f], [x[h] for x in f]))
        o += s * (h + 1)
    return (w1, b1), ([x[:h] for x in sel], [x[h] for x in sel]), subredes


def adelante_logica(p, x):
    """(sumas de la subred elegida, subred elegida), como RedLogica.cpp."""
    (w1, b1), (ws, bs), subredes = partes_logica(p)
    h = [max(0, b + sum(w * xi for w, xi in zip(fila, x))) for fila, b in zip(w1, b1)]
    g = [b + sum(w * hj for w, hj in zip(fila, h)) for fila, b in zip(ws, bs)]
    elegida = max(range(len(g)), key=lambda k: (g[k], -k))
    w2, b2 = subredes[elegida]
    return [b + sum(w * hj for w, hj in zip(fila, h)) for fila, b in zip(w2, b2)], elegida


def a_jerarquica(plana):
    """Red jerárquica que decide igual que la red plana: selector a cero (gana
    la subred 0) y todas las subredes copia de la capa de salida."""
    e, h, s = LOGICA
    n = num_parametros(LOGICA)
    assert len(plana) == n
    return list(plana) + [0] * (SUBREDES * (h + 1)) + list(plana[h * (e + 1):]) * (SUBREDES - 1)


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
    sumas = adelante(p, LOGICA, x) if len(p) == num_parametros(LOGICA) else adelante_logica(p, x)[0]
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


def escribir_logica_jerarquica(p):
    e, h, s = LOGICA
    (w1, b1), (ws, bs), subredes = partes_logica(p)
    fila = lambda f, b, nota="": "    " + ", ".join(f"{v:4d}" for v in f) + f",  {b:4d}," + (f"   // {nota}" if nota else "")
    lin = [fila(f, b) for f, b in zip(w1, b1)]
    lin.append("    // subred 0 (una fila por acción)")
    lin += [fila(f, b, n) for n, f, b in zip(ACCIONES, *subredes[0])]
    lin.append("    // selector: una fila por subred (gana la mayor)")
    lin += [fila(f, b, f"subred {k}") for k, (f, b) in enumerate(zip(ws, bs))]
    for k in range(1, SUBREDES):
        lin.append(f"    // subred {k}")
        lin += [fila(f, b, n) for n, f, b in zip(ACCIONES, *subredes[k])]
    TABLA_LOGICA.write_text(f"""// TablaRedLogica.h — Pesos de la red de lógica (los escribe simulacion/entrenar_redes.py)
// Red principal + subredes, enteros int8. Red principal: {e} entradas -> {h}
// neuronas ReLU (entradas: {", ".join(ENTRADAS_LOGICA)}) y un selector que elige
// una de las {SUBREDES} subredes; la subred elegida da una suma por acción y gana la
// mayor. Cada fila: los pesos de una neurona y, al final, su sesgo. Orden: capa
// oculta, subred 0, selector, subredes 1..{SUBREDES - 1}.
#ifndef TABLA_RED_LOGICA_H
#define TABLA_RED_LOGICA_H

#include "estrategia/TablaRed.h"

namespace redlogica {{

constexpr int NumEntradas = {e};
constexpr int NumOcultas = {h};
constexpr int NumSalidas = {s};
constexpr int NumSubredes = {SUBREDES};
constexpr int NumParametros = NumOcultas * (NumEntradas + 1) + NumSubredes * NumSalidas * (NumOcultas + 1) +
                              NumSubredes * (NumOcultas + 1);

TABLA_RED(Parametros, NumParametros) = {{
""" + "\n".join(lin) + """
};

}

#endif
""")


def escribir_logica(p):
    # El firmware solo lee la forma jerárquica: una red plana se convierte (mismas decisiones)
    escribir_logica_jerarquica(p if len(p) == num_parametros_logica() else a_jerarquica(p))


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
    if len(logica) == num_parametros_logica():
        # Se aplica a la red plana (capa oculta + subred 0) y se rehace la jerarquía
        return a_jerarquica(propuesta_esquiva(logica[:num_parametros(LOGICA)], fuerza))
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
