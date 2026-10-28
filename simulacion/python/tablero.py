#!/usr/bin/env python3
"""Tablero del entrenamiento: candidatos, redes neuronales y simulaciones.

Lee las baterías de simulacion/resultados/candidatos/*.json (bateria.py --json),
el registro de DAgger y las tablas del firmware, y escribe
simulacion/resultados/tablero.html, que se recarga sola cada minuto.

  python3 -m simulacion.python.tablero            # una vez
  python3 -m simulacion.python.tablero --cada 60  # regenerar mientras se entrena
"""
import argparse
import html
import json
import time

from . import redes
from .entrenar_jerarquica import error_de
from .entrenar_politica import ACCIONES, accion_manual, describir

RAIZ = redes.RAIZ
RES = RAIZ / "simulacion" / "resultados"
CAND = RES / "candidatos"
SALIDA = RES / "tablero.html"


def pct(x, n):
    return 100 * x / n if n else 0


def error(t):
    return error_de(t["victorias"], t["caidas"], t["caidas_propias"], t["combates"])


def candidatos():
    out = []
    for ruta in sorted(CAND.glob("*.json")):
        try:
            out.append((ruta.stem, json.loads(ruta.read_text())))
        except (OSError, ValueError):
            pass  # batería a medio escribir: aparece en la próxima recarga
    return out


def barras(filas):
    if not filas:
        return "<p>Aún no hay candidatos evaluados.</p>"
    alto, mx = 26, max(e for _, e, _ in filas) or 1
    svg = [f'<svg viewBox="0 0 760 {alto * len(filas) + 10}" role="img" aria-label="Error por candidato">']
    for i, (nombre, e, mejor) in enumerate(filas):
        y, w = 5 + i * alto, 500 * e / mx
        svg.append(f'<text x="200" y="{y + 16}" text-anchor="end">{html.escape(nombre)}</text>'
                   f'<rect x="210" y="{y + 3}" width="{w:.1f}" height="{alto - 8}" class="{"mejor" if mejor else "barra"}"/>'
                   f'<text x="{216 + w:.1f}" y="{y + 16}">{e:.1f}</text>')
    return "".join(svg) + "</svg>"


def tabla_candidatos(cands):
    if not cands:
        return "", ""
    base = dict(cands).get("base")
    filas = sorted(((n, error(d["total"]), d) for n, d in cands), key=lambda f: f[1])
    mejor = filas[0][0]
    cab = "<tr><th>candidato</th><th>combates</th><th>gana %</th><th>se cae %</th><th>caídas propias</th><th>error</th>" + \
          "".join(f"<th>modo {m} gana/cae</th>" for m in "1234") + "<th>vs base</th></tr>"
    cuerpo = []
    for n, e, d in filas:
        t = d["total"]
        modos = "".join(f"<td>{pct(d['por_modo'][m]['victorias'], d['por_modo'][m]['combates']):.0f}/"
                        f"{pct(d['por_modo'][m]['caidas'], d['por_modo'][m]['combates']):.0f}</td>"
                        if m in d["por_modo"] else "<td>–</td>" for m in "1234")
        vs = f"{error(base['total']) - e:+.1f}" if base else "–"
        cuerpo.append(f'<tr{" class=sel" if n == mejor else ""}><td>{html.escape(n)}</td><td>{t["combates"]}</td>'
                      f'<td>{pct(t["victorias"], t["combates"]):.1f}</td><td>{pct(t["caidas"], t["combates"]):.1f}</td>'
                      f'<td>{t["caidas_propias"]}</td><td>{e:.2f}</td>{modos}<td>{vs}</td></tr>')
    rivales = ""
    if base:
        d = dict(cands)[mejor]
        rivales = "<tr><th>rival</th><th>base gana/cae %</th><th>" + html.escape(mejor) + " gana/cae %</th></tr>" + "".join(
            f"<tr><td>{html.escape(r)}</td><td>{pct(b['victorias'], b['combates']):.0f}/{pct(b['caidas'], b['combates']):.0f}</td>"
            f"<td>{pct(d['por_rival'][r]['victorias'], d['por_rival'][r]['combates']):.0f}/"
            f"{pct(d['por_rival'][r]['caidas'], d['por_rival'][r]['combates']):.0f}</td></tr>"
            for r, b in base["por_rival"].items() if r in d["por_rival"])
    graf = barras([(n, e, n == mejor) for n, e, _ in filas])
    return (f"<p>Mejor: <b>{html.escape(mejor)}</b> (error {filas[0][1]:.2f}; menor es mejor).</p>{graf}"
            f'<div class="tabla"><table>{cab}{"".join(cuerpo)}</table></div>', rivales)


def tabla_politicas():
    tablas = {"escrita a mano": [accion_manual(e) for e in range(64)]}
    for nombre, archivo in [("DAgger + MPC", "TablaPolitica_dagger.h"), ("ε-greedy", "TablaPolitica_greedy.h")]:
        ruta = CAND / archivo
        if ruta.exists():
            tablas[nombre] = redes.leer_tabla(ruta)
    if len(tablas) == 1:
        return "<p>Aún no hay tablas entrenadas.</p>"
    cab = "<tr><th>estado (sensores que ven al rival)</th>" + "".join(f"<th>{html.escape(n)}</th>" for n in tablas) + "</tr>"
    filas = [f"<tr><td>{html.escape(describir(e))}</td>" + "".join(
        f'<td{" class=cambio" if t[e] != tablas["escrita a mano"][e] else ""}>{ACCIONES[t[e]]}</td>' for t in tablas.values()) + "</tr>"
        for e in range(64) if any(t[e] != tablas["escrita a mano"][e] for t in tablas.values())]
    return f'<p>Solo los estados donde alguna tabla cambia la acción escrita a mano ({len(filas)} de 64).</p>' \
           f'<div class="tabla"><table>{cab}{"".join(filas)}</table></div>'


def mapa(matriz, filas, columnas, titulo):
    """Mapa de calor de una capa: azul positivo, naranja negativo."""
    c, f = 22, 16
    mx = max(abs(v) for fila in matriz for v in fila) or 1
    svg = [f'<svg viewBox="0 0 {90 + c * len(columnas)} {40 + f * len(filas)}" role="img" aria-label="{titulo}">']
    for j, n in enumerate(columnas):
        svg.append(f'<text x="{90 + c * j + c / 2}" y="30" text-anchor="middle">{html.escape(n)}</text>')
    for i, (n, fila) in enumerate(zip(filas, matriz)):
        svg.append(f'<text x="84" y="{48 + f * i}" text-anchor="end">{html.escape(n)}</text>')
        for j, v in enumerate(fila):
            svg.append(f'<rect x="{90 + c * j}" y="{36 + f * i}" width="{c - 1}" height="{f - 1}" '
                       f'fill="var(--{"a" if v >= 0 else "c"})" opacity="{0.1 + 0.9 * abs(v) / mx:.2f}"><title>{v}</title></rect>')
    return "".join(svg) + "</svg>"


def seccion_redes():
    e, h, s = redes.LOGICA
    try:
        logica = redes.leer_tabla(redes.TABLA_LOGICA)
        (w1, _), _, _ = redes.partes_logica(logica)
        control = redes.leer_tabla(redes.TABLA_CONTROL)
        cw1, _, _, _ = redes.separar(control, redes.CONTROL)
    except (OSError, IndexError, ValueError) as err:
        return f"<p>No se pudieron leer las tablas de las redes: {html.escape(str(err))}</p>"
    ce, ch, cs = redes.CONTROL
    neuronas = [f"n{i}" for i in range(h)]
    return (f"<p>Red de lógica: {e} entradas → {h} neuronas ReLU → selector de {redes.SUBREDES} subredes → {s} acciones "
            f"({len(logica)} pesos int8). Red de control: {ce} → {ch} ReLU → {cs} ({len(control)} pesos int8). "
            f"Se entrenan con <code>entrenar_redes.py</code> (OpenAI-ES) y <code>entrenar_jerarquica.py</code> (sep-CMA-ES).</p>"
            f"<h3>Lógica: primera capa</h3>{mapa(w1, neuronas, redes.ENTRADAS_LOGICA, 'Pesos de la primera capa de la red de lógica')}"
            f"<h3>Control: primera capa</h3>{mapa(cw1, [f'n{i}' for i in range(ch)], redes.ENTRADAS_CONTROL, 'Pesos de la primera capa de la red de control')}")


def registro(nombre):
    ruta = RAIZ / "simulacion" / "build" / nombre
    if not ruta.exists():
        return "<p>Aún no ha empezado.</p>"
    lineas = [l for l in ruta.read_text().splitlines() if l.strip()][-12:]
    return "<pre>" + html.escape("\n".join(lineas)) + "</pre>"


def escribir():
    cands = candidatos()
    resumen, rivales = tabla_candidatos(cands)
    SALIDA.write_text(f"""<!doctype html><html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1"><meta http-equiv="refresh" content="60">
<title>Tablero de entrenamiento</title><style>
:root {{ --bg:#f6f7f9; --fg:#16191d; --mut:#5d6670; --rule:#d5d9de; --a:#1f6fb2; --b:#19744a; --c:#c2541b; }}
@media (prefers-color-scheme: dark) {{ :root {{ --bg:#121418; --fg:#e8eaed; --mut:#9aa3ad; --rule:#2e333a; --a:#5fa8e6; --b:#4cc38a; --c:#ef8a4f; }} }}
body {{ background:var(--bg); color:var(--fg); font:14px/1.5 system-ui, sans-serif; margin:0 auto; padding:16px; max-width:1100px; }}
h1 {{ font-size:22px; margin:0 0 4px }} h2 {{ font-size:17px; margin:26px 0 6px }} h3 {{ font-size:14px; margin:14px 0 4px }} p {{ color:var(--mut); margin:4px 0 }}
.tabla {{ overflow-x:auto }} table {{ border-collapse:collapse; font-variant-numeric:tabular-nums; font-size:13px }}
th, td {{ border-bottom:1px solid var(--rule); padding:4px 8px; text-align:right; white-space:nowrap }}
th {{ color:var(--mut); font-weight:600 }} td:first-child, th:first-child {{ text-align:left }}
tr.sel td {{ font-weight:700; color:var(--b) }} td.cambio {{ color:var(--c); font-weight:600 }}
svg {{ width:100%; height:auto; max-width:760px }} svg text {{ fill:var(--mut); font-size:11px }}
.barra {{ fill:var(--a); opacity:.55 }} .mejor {{ fill:var(--b) }} pre {{ background:var(--rule); padding:8px; overflow-x:auto; font-size:12px }}
</style></head><body>
<h1>Tablero de entrenamiento — Boldenona</h1>
<p>Actualizado {time.strftime("%Y-%m-%d %H:%M:%S")}. Error = (100 − % victorias) + 2 × % caídas + 5 × % caídas propias (0 = perfecto).
Colocaciones de Rumania RU1–RU4, 11 rivales, 12 factores. La página se recarga cada minuto.</p>
<h2>Candidatos (DAgger, MPC, ε-greedy, PID con anti-windup)</h2>{resumen}
<h3>Por rival: base frente al mejor</h3><div class="tabla"><table>{rivales}</table></div>
<h2>Tabla de combate aprendida</h2>{tabla_politicas()}
<h2>Redes neuronales del firmware</h2>{seccion_redes()}
<h2>Registros</h2><h3>DAgger + MPC</h3>{registro("dagger.txt")}<h3>ε-greedy</h3>{registro("greedy.txt")}<h3>Rombo</h3>{registro("rombo.txt")}
<h2>Simulaciones</h2>
<p>Simulador 3D en el navegador: <code>scripts/correr.sh web</code> (abre http://localhost:5173). Combates sueltos con trayectoria:
<code>python3 -m simulacion.python.simular --tray simulacion/build/tray.csv</code>.</p>
</body></html>""")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--cada", type=float, default=0, help="regenerar cada estos segundos (0 = una vez)")
    a = ap.parse_args()
    while True:
        escribir()
        if a.cada <= 0:
            break
        time.sleep(a.cada)
    print(SALIDA)


if __name__ == "__main__":
    main()
