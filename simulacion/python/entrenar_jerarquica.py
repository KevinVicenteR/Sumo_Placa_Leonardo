#!/usr/bin/env python3
"""Entrena la red principal + subredes (red de lógica) y la red de control con
sep-CMA-ES en el simulador, mostrando la tabla del error de entrenamiento.

Algoritmo: sep-CMA-ES (Ros y Hansen, 2008), CMA-ES con covarianza diagonal:
adapta el tamaño de paso σ y la escala de cada peso, con coste lineal en el
número de pesos (≈740). Cada candidato se evalúa con sus pesos redondeados a
int8, igual que en el robot, y todos los de una generación en los mismos
combates (mismas semillas).

Error de una casilla (round × rival × factor), en puntos (0 = perfecto):
    error = (100 − % victorias) + 2 × % caídas + 5 × % caídas propias
El error de un candidato es la media de sus casillas.

Para bajar el error:
  - Solo se entrena en las casillas que se pueden mejorar (--base: batería de
    referencia): fuera el rival de 500 g y las casillas que la red actual ya
    gana siempre o pierde siempre (p. ej. las dos llantas gastadas).
  - Cada generación usa una muestra nueva de esas casillas; todos los candidatos
    de la generación, en los mismos combates.
  - Validación pareada: la red de partida y los candidatos se miden en los
    mismos combates (mismas semillas). Un candidato solo se acepta si baja el
    error y no se cae más que la red de partida.
  - σ con tope (--sigma-max). Si la validación no mejora en --paciencia
    validaciones, se reinicia CMA-ES desde el mejor candidato con el σ inicial.

Salida (simulacion/resultados/):
  entrenamiento.csv   una fila por generación (error de entrenamiento y validación)
  entrenamiento.html  tabla y curva del error, por rival y por modo
  candidatos_jerarquica/  los pesos de cada validación (formato --red)

  python3 -m simulacion.python.entrenar_jerarquica --horas 3.5
"""
import argparse
import csv
import math
import os
import random
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

from . import simular
from . import bateria as B
from . import redes as R
from .entrenar_redes import combatir

RESULTADOS = Path(__file__).resolve().parents[1] / "resultados"
CSV = RESULTADOS / "entrenamiento.csv"
HTML = RESULTADOS / "entrenamiento.html"
CANDIDATOS = RESULTADOS / "candidatos_jerarquica"
RIVALES_NORMALES = ["quieto", "deambula", "embiste", "flanquea", "tornado", "contragolpe",
                    "woodpecker", "esquivador"]


# Pesos del error (--peso-caida, --peso-propia) y rivales fuera del entrenamiento
PESO_CAIDA, PESO_PROPIA = 2, 5
EXCLUIDOS = {"pesado 500 g"}


def error_de(v, c, p, n, sin_rival=False):
    """Sin rival no se puede ganar: solo cuentan las caídas."""
    if not n:
        return 0.0
    return (0 if sin_rival else 100 - 100 * v / n) + PESO_CAIDA * 100 * c / n + PESO_PROPIA * 100 * p / n


def error_casilla(res, i, j, casilla):
    v, c, n, p = res[(i, j)][:4]
    return error_de(v, c, p, n, casilla[1][1] == "ninguno")


def modo_de(ronda):
    return B.MODO_DE_ROUND[ronda]


class SepCMAES:
    """sep-CMA-ES: CMA-ES con matriz de covarianza diagonal (Ros y Hansen, 2008)."""

    def __init__(self, media, sigma, lam, rng):
        self.n = n = len(media)
        self.m = list(media)
        self.sigma = sigma
        self.lam = lam
        self.mu = lam // 2
        w = [math.log(self.mu + 0.5) - math.log(i + 1) for i in range(self.mu)]
        s = sum(w)
        self.w = [x / s for x in w]
        self.mueff = 1 / sum(x * x for x in self.w)
        self.cs = (self.mueff + 2) / (n + self.mueff + 5)
        self.ds = 1 + 2 * max(0, math.sqrt((self.mueff - 1) / (n + 1)) - 1) + self.cs
        self.cc = (4 + self.mueff / n) / (n + 4 + 2 * self.mueff / n)
        c1 = 2 / ((n + 1.3) ** 2 + self.mueff)
        cmu = min(1 - c1, 2 * (self.mueff - 2 + 1 / self.mueff) / ((n + 2) ** 2 + self.mueff))
        # En la versión separable las tasas de aprendizaje se multiplican por (n+2)/3
        self.c1 = c1 * (n + 2) / 3
        self.cmu = min(1 - self.c1, cmu * (n + 2) / 3)
        self.C = [1.0] * n
        self.ps = [0.0] * n
        self.pc = [0.0] * n
        self.chiN = math.sqrt(n) * (1 - 1 / (4 * n) + 1 / (21 * n * n))
        self.rng = rng
        self.gen = 0

    def pedir(self):
        """λ candidatos: (z, x) con x = m + σ·√C·z."""
        self.z = [[self.rng.gauss(0, 1) for _ in range(self.n)] for _ in range(self.lam)]
        return [[mi + self.sigma * math.sqrt(ci) * zi for mi, ci, zi in zip(self.m, self.C, z)] for z in self.z]

    def actualizar(self, errores):
        """Minimiza el error: los μ mejores mueven la media y adaptan σ y C."""
        self.gen += 1
        orden = sorted(range(self.lam), key=lambda i: errores[i])[:self.mu]
        zw = [sum(self.w[k] * self.z[i][d] for k, i in enumerate(orden)) for d in range(self.n)]
        self.m = [mi + self.sigma * math.sqrt(ci) * z for mi, ci, z in zip(self.m, self.C, zw)]
        a = math.sqrt(self.cs * (2 - self.cs) * self.mueff)
        self.ps = [(1 - self.cs) * p + a * z for p, z in zip(self.ps, zw)]
        norma = math.sqrt(sum(p * p for p in self.ps))
        hs = norma / math.sqrt(1 - (1 - self.cs) ** (2 * self.gen)) / self.chiN < 1.4 + 2 / (self.n + 1)
        b = math.sqrt(self.cc * (2 - self.cc) * self.mueff)
        self.pc = [(1 - self.cc) * p + (b * math.sqrt(c) * z if hs else 0) for p, c, z in zip(self.pc, self.C, zw)]
        for d in range(self.n):
            rmu = sum(self.w[k] * self.C[d] * self.z[i][d] ** 2 for k, i in enumerate(orden))
            self.C[d] = ((1 - self.c1 - self.cmu) * self.C[d] + self.c1 * self.pc[d] ** 2 + self.cmu * rmu)
            self.C[d] = min(max(self.C[d], 1e-4), 1e4)
        self.sigma *= math.exp((self.cs / self.ds) * (norma / self.chiN - 1))
        self.sigma = min(max(self.sigma, 0.2), 20.0)


def redondear(x):
    return [max(-127, min(127, int(round(v)))) for v in x]


class Evaluador:
    def __init__(self, binario, carpeta):
        self.binario = binario
        self.carpeta = Path(carpeta)
        self.contador = 0
        self.ex = ThreadPoolExecutor(os.cpu_count())

    def archivo(self, pesos, nl):
        self.contador += 1
        ruta = self.carpeta / f"c{self.contador}.red"
        R.guardar_pesos(ruta, pesos[:nl], pesos[nl:])
        return ruta

    def resultados(self, candidatos, casillas, n, semilla, nl):
        """{(i, j): (victorias, caídas, combates, propias, ...)} del candidato i en la casilla j."""
        rutas = [self.archivo(c, nl) for c in candidatos]
        tareas = [(i, j) for i in range(len(rutas)) for j in range(len(casillas))]
        res = self.ex.map(lambda t: (t, combatir(self.binario, rutas[t[0]], casillas[t[1]], n, semilla)), tareas)
        salida = {k: v for k, v in res}
        for ruta in rutas:
            ruta.unlink()
        return salida


def agrupar(res, casillas, i, clave):
    """Error por grupo (rival o modo) del candidato i."""
    grupos = {}
    for j, c in enumerate(casillas):
        grupos.setdefault(clave(c), []).append(error_casilla(res, i, j, c))
    return {g: sum(e) / len(e) for g, e in grupos.items()}


def error_total(res, casillas, i):
    errores = [error_casilla(res, i, j, c) for j, c in enumerate(casillas)]
    return sum(errores) / len(errores)


def escribir_html(filas, validaciones, por_rival, por_modo, inicio, mejor):
    """Página con la tabla y la curva del error (se regenera en cada validación)."""
    ancho, alto, m = 760, 260, 36
    puntos = [(f["gen"], f["err_media"], f["err_mejor"]) for f in filas]
    vals = [(v["gen"], v["err_val"]) for v in validaciones]
    todos = [p[1] for p in puntos] + [p[2] for p in puntos] + [v[1] for v in vals] or [1]
    ymax, ymin = max(todos), min(todos)
    if ymax - ymin < 1:
        ymax += 0.5; ymin -= 0.5
    gmax = max([p[0] for p in puntos] + [1])
    X = lambda g: m + (ancho - 2 * m) * g / gmax
    Y = lambda e: alto - m - (alto - 2 * m) * (e - ymin) / (ymax - ymin)
    linea = lambda pts: " ".join(f"{X(g):.1f},{Y(e):.1f}" for g, e in pts)
    eje_y = "".join(f'<text x="{m - 6}" y="{Y(ymin + (ymax - ymin) * k / 4) + 4:.1f}" text-anchor="end">'
                    f'{ymin + (ymax - ymin) * k / 4:.1f}</text>' for k in range(5))
    svg = f"""<svg viewBox="0 0 {ancho} {alto}" role="img" aria-label="Curva del error de entrenamiento">
<line x1="{m}" y1="{alto - m}" x2="{ancho - m}" y2="{alto - m}" class="eje"/>
<line x1="{m}" y1="{m}" x2="{m}" y2="{alto - m}" class="eje"/>{eje_y}
<text x="{ancho - m}" y="{alto - 8}" text-anchor="end">generación</text>
<polyline points="{linea([(g, e) for g, e, _ in puntos])}" class="media"/>
<polyline points="{linea([(g, e) for g, _, e in puntos])}" class="mejor"/>
<polyline points="{linea(vals)}" class="val"/>
{''.join(f'<circle cx="{X(g):.1f}" cy="{Y(e):.1f}" r="3" class="valp"/>' for g, e in vals)}
</svg>"""
    cab_r = sorted(por_rival[-1]["errores"]) if por_rival else []
    cab_m = sorted(por_modo[-1]["errores"]) if por_modo else []
    fila_v = lambda v, d: (f"<tr><td>{v['gen']}</td><td>{v['etapa']}</td><td>{v['sigma']:.2f}</td>"
                           f"<td>{v['err_val']:.2f}</td><td>{v['gana']:.1f} %</td><td>{v['cae']:.1f} %</td>"
                           f"<td>{v['propias']:.2f} %</td>"
                           + "".join(f"<td>{d['errores'].get(k, 0):.1f}</td>" for k in cab_r) + "</tr>")
    tabla_val = "".join(fila_v(v, d) for v, d in zip(validaciones, por_rival))
    tabla_modo = "".join("<tr><td>" + str(v["gen"]) + "</td>" + "".join(f"<td>{d['errores'].get(k, 0):.1f}</td>" for k in cab_m) + "</tr>"
                         for v, d in zip(validaciones, por_modo))
    ultimas = filas[-15:]
    tabla_gen = "".join(f"<tr><td>{f['gen']}</td><td>{f['etapa']}</td><td>{f['sigma']:.2f}</td><td>{f['err_mejor']:.2f}</td>"
                        f"<td>{f['err_media']:.2f}</td><td>{f['segundos']:.0f} s</td></tr>" for f in ultimas)
    HTML.write_text(f"""<!doctype html><html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1"><meta http-equiv="refresh" content="60">
<title>Error de entrenamiento</title><style>
:root {{ --bg:#f6f7f9; --fg:#16191d; --mut:#5d6670; --rule:#d5d9de; --a:#1f6fb2; --b:#19744a; --c:#c2541b; }}
@media (prefers-color-scheme: dark) {{ :root {{ --bg:#121418; --fg:#e8eaed; --mut:#9aa3ad; --rule:#2e333a; --a:#5fa8e6; --b:#4cc38a; --c:#ef8a4f; }} }}
body {{ background:var(--bg); color:var(--fg); font:14px/1.5 system-ui, sans-serif; margin:0 auto; padding:16px; max-width:1100px; }}
h1 {{ font-size:22px; margin:0 0 4px }} h2 {{ font-size:16px; margin:22px 0 6px }} p {{ color:var(--mut); margin:4px 0 }}
.tabla {{ overflow-x:auto }} table {{ border-collapse:collapse; font-variant-numeric:tabular-nums; font-size:13px }}
th, td {{ border-bottom:1px solid var(--rule); padding:4px 8px; text-align:right; white-space:nowrap }}
th {{ color:var(--mut); font-weight:600 }} svg {{ width:100%; height:auto; max-width:760px }}
svg text {{ fill:var(--mut); font-size:11px }} .eje {{ stroke:var(--rule) }}
polyline {{ fill:none; stroke-width:2 }} .media {{ stroke:var(--a); opacity:.45 }} .mejor {{ stroke:var(--a) }}
.val {{ stroke:var(--c) }} .valp {{ fill:var(--c) }} .ley span {{ margin-right:14px }} .ley b {{ display:inline-block; width:12px; height:3px; vertical-align:middle; margin-right:4px }}
</style></head><body>
<h1>Error de entrenamiento: red principal + subredes</h1>
<p>Error = (100 − % victorias) + 2 × % caídas + 5 × % caídas propias (0 = perfecto). sep-CMA-ES, colocaciones de Rumania, 11 rivales. Error de validación de partida: <b>{inicio:.2f}</b>; mejor: <b>{mejor:.2f}</b>. La página se recarga cada minuto.</p>
<p class="ley"><span><b style="background:var(--a)"></b>mejor candidato (entrenamiento)</span><span><b style="background:var(--a);opacity:.45"></b>media de la generación</span><span><b style="background:var(--c)"></b>validación (combates nuevos)</span></p>
{svg}
<h2>Validaciones: error total y por rival</h2>
<div class="tabla"><table><thead><tr><th>gen.</th><th>etapa</th><th>σ</th><th>error</th><th>gana</th><th>cae</th><th>propias</th>{''.join(f'<th>{k}</th>' for k in cab_r)}</tr></thead><tbody>{tabla_val}</tbody></table></div>
<h2>Validaciones: error por modo</h2>
<div class="tabla"><table><thead><tr><th>gen.</th>{''.join(f'<th>modo {k}</th>' for k in cab_m)}</tr></thead><tbody>{tabla_modo}</tbody></table></div>
<h2>Últimas generaciones</h2>
<div class="tabla"><table><thead><tr><th>gen.</th><th>etapa</th><th>σ</th><th>error mejor</th><th>error medio</th><th>tiempo</th></tr></thead><tbody>{tabla_gen}</tbody></table></div>
</body></html>""")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--horas", type=float, default=3.5, help="tiempo máximo de entrenamiento")
    ap.add_argument("--lambda", dest="lam", type=int, default=16, help="candidatos por generación")
    ap.add_argument("--sigma", type=float, default=2.0, help="paso inicial (unidades int8)")
    ap.add_argument("--n", type=int, default=2, help="combates por casilla en el entrenamiento")
    ap.add_argument("--muestra", type=int, default=60, help="casillas por generación (etapa 2)")
    ap.add_argument("--validar", type=int, default=5, help="generaciones entre validaciones")
    ap.add_argument("--n-validar", type=int, default=3, help="combates por casilla en la validación")
    ap.add_argument("--paciencia", type=int, default=4, help="validaciones sin mejora antes de adaptar")
    ap.add_argument("--inicio", help="pesos de partida (.red) en vez de las tablas del firmware")
    ap.add_argument("--fijas", action="store_true",
                    help="cada generación incluye también las casillas ideales (sin el rival de 500 g)")
    ap.add_argument("--peso-caida", type=float, default=2, help="peso de cada %% de caídas en el error")
    ap.add_argument("--peso-propia", type=float, default=5, help="peso de cada %% de caídas propias")
    ap.add_argument("--incluir-pesado", action="store_true",
                    help="entrena también contra el rival de 500 g (robot igual de pesado)")
    ap.add_argument("--sigma-max", type=float, default=5.0, help="tope de σ")
    ap.add_argument("--base", default=str(RESULTADOS / "rumania11_antes_30000.json"),
                    help="batería de referencia (JSON de bateria.py) para elegir las casillas mejorables")
    a = ap.parse_args()

    B.ROUNDS = B.ROUNDS_RUMANIA
    global PESO_CAIDA, PESO_PROPIA, EXCLUIDOS
    PESO_CAIDA, PESO_PROPIA = a.peso_caida, a.peso_propia
    EXCLUIDOS = set() if a.incluir_pesado else EXCLUIDOS
    RESULTADOS.mkdir(exist_ok=True)
    CANDIDATOS.mkdir(exist_ok=True)
    binario = simular.compilar({})
    nl = R.num_parametros_logica()
    inicio = ([int(x) for x in Path(a.inicio).read_text().split()] if a.inicio
              else R.leer_tabla(R.TABLA_LOGICA) + R.leer_tabla(R.TABLA_CONTROL))
    assert len(inicio) == nl + R.num_parametros(R.CONTROL), "la tabla de lógica no es jerárquica"
    rng = random.Random(2026)
    es = SepCMAES([float(x) for x in inicio], a.sigma, a.lam, rng)

    todas = [(r, rv, f) for f in B.FACTORES for r in B.ROUNDS for rv in B.RIVALES]
    # Casillas mejorables según la batería de referencia
    import json
    base = {(c["round"], c["rival"], c["factor"]): c for c in json.load(open(a.base))["casillas"]}

    def mejorable(c):
        b = base[(c[0][0], c[1][0], c[2][0])]
        if c[1][0] in EXCLUIDOS or b["caidas"] >= b["combates"] - 1:
            return False
        if c[1][1] == "ninguno":
            return b["caidas"] > 0
        return b["caidas"] > 0 or b["victorias"] < b["combates"] - 1

    entreno = [c for c in todas if mejorable(c)]
    fijas = [c for c in todas if a.fijas and c[2] is B.FACTORES[0] and c[1][0] not in EXCLUIDOS]
    # Validación fija: todas las casillas salvo los rivales excluidos, semillas reservadas
    val_casillas = [c for c in todas if c[1][0] not in EXCLUIDOS]
    print(f"{len(entreno)} casillas mejorables de {len(todas)}; validación en {len(val_casillas)}", flush=True)
    filas, validaciones, por_rival, por_modo = [], [], [], []
    etapa, sin_mejora = 2, 0
    t_fin = time.time() + a.horas * 3600

    with tempfile.TemporaryDirectory() as carpeta, open(CSV, "w", newline="") as fcsv:
        esc = csv.writer(fcsv)
        esc.writerow(["generacion", "etapa", "sigma", "error_mejor", "error_medio", "error_validacion",
                      "gana_val", "cae_val", "propias_val", "segundos"])
        ev = Evaluador(binario, carpeta)

        def validar(pesos, gen):
            res = ev.resultados([pesos], val_casillas, a.n_validar, 500000, nl)
            J = range(len(val_casillas))
            v = sum(res[(0, j)][0] for j in J); k = sum(res[(0, j)][1] for j in J)
            n = sum(res[(0, j)][2] for j in J); p = sum(res[(0, j)][3] for j in J)
            err = error_total(res, val_casillas, 0)
            r = agrupar(res, val_casillas, 0, lambda c: c[1][0])
            m = agrupar(res, val_casillas, 0, lambda c: modo_de(c[0][0]))
            return err, 100 * v / n, 100 * k / n, 100 * p / n, r, m

        err0, g0, c0, p0, r0, m0 = validar(inicio, 0)
        mejor = (err0, inicio)
        caidas0 = c0
        validaciones.append({"gen": 0, "etapa": 1, "sigma": es.sigma, "err_val": err0, "gana": g0, "cae": c0, "propias": p0})
        por_rival.append({"errores": r0}); por_modo.append({"errores": m0})
        print(f"partida: error de validación {err0:.2f} (gana {g0:.1f} %, cae {c0:.1f} %, propias {p0:.2f} %)", flush=True)
        escribir_html(filas, validaciones, por_rival, por_modo, err0, err0)

        gen = 0
        while time.time() < t_fin:
            gen += 1
            t0 = time.time()
            casillas = fijas + rng.sample([c for c in entreno if c not in fijas], min(a.muestra, len(entreno)))
            xs = es.pedir()
            cand = [redondear(x) for x in xs]
            res = ev.resultados(cand, casillas, a.n, 1000 + 37 * gen, nl)
            errores = [error_total(res, casillas, i) for i in range(len(cand))]
            es.actualizar(errores)
            es.sigma = min(es.sigma, a.sigma_max)
            fila = {"gen": gen, "etapa": etapa, "sigma": es.sigma, "err_mejor": min(errores),
                    "err_media": sum(errores) / len(errores), "segundos": time.time() - t0}
            filas.append(fila)
            ev_val = ""
            if gen % a.validar == 0:
                actual = redondear(es.m)
                mejor_gen = cand[min(range(len(cand)), key=lambda i: errores[i])]
                propuestas = [actual, mejor_gen]
                evals = [(p,) + validar(p, gen) for p in propuestas]
                p, err, g, c, pr, r, m = min(evals, key=lambda e: e[1])
                for i, (q, *_rest) in enumerate(evals):
                    R.guardar_pesos(CANDIDATOS / f"g{gen:04d}_{i}_err{_rest[0]:.2f}.red", q[:nl], q[nl:])
                marca = ""
                if err < mejor[0] - 0.05 and c <= caidas0:
                    mejor = (err, p)
                    sin_mejora = 0
                    marca = "  <- mejor"
                    R.guardar_pesos(RESULTADOS / "jerarquica_mejor.red", p[:nl], p[nl:])
                else:
                    sin_mejora += 1
                    if sin_mejora >= a.paciencia:
                        sin_mejora = 0
                        es = SepCMAES([float(x) for x in mejor[1]], a.sigma, a.lam, rng)
                        print(f"   adaptación: reinicio desde el mejor (σ = {a.sigma:.2f})", flush=True)
                validaciones.append({"gen": gen, "etapa": etapa, "sigma": es.sigma, "err_val": err, "gana": g, "cae": c, "propias": pr})
                por_rival.append({"errores": r}); por_modo.append({"errores": m})
                escribir_html(filas, validaciones, por_rival, por_modo, err0, mejor[0])
                ev_val = f"{err:.3f}"
                peor = sorted(r.items(), key=lambda kv: -kv[1])[:3]
                print(f"== validación {gen}: error {err:.2f} (gana {g:.1f} %, cae {c:.1f} %, propias {pr:.2f} %) "
                      f"| peores: {', '.join(f'{k} {v:.1f}' for k, v in peor)}{marca}", flush=True)
            esc.writerow([gen, etapa, f"{es.sigma:.3f}", f"{fila['err_mejor']:.3f}", f"{fila['err_media']:.3f}", ev_val,
                          "", "", "", f"{fila['segundos']:.1f}"])
            fcsv.flush()
            print(f"[{gen}] etapa {etapa} σ {es.sigma:.2f} | error mejor {fila['err_mejor']:.2f} medio {fila['err_media']:.2f} "
                  f"({fila['segundos']:.0f} s)", flush=True)

    print(f"\nFin: error de validación de partida {err0:.2f}, mejor {mejor[0]:.2f}. Pesos en resultados/jerarquica_mejor.red")


if __name__ == "__main__":
    main()
