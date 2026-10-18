import { useEffect, useRef, useState, useSyncExternalStore } from "react";
import { createRoot } from "react-dom/client";
import { Simulador, type Pose } from "./simulador";
import { crearEscena } from "./escena";
import { factores } from "./factores";
import type { Firmware } from "./wasm";
import "./estilos.css";

type Opcion = readonly [string, string];
function Selector({
  titulo,
  valor,
  opciones,
  cambiar,
  deshabilitado = false,
}: {
  titulo: string;
  valor: string;
  opciones: readonly Opcion[];
  cambiar: (v: string) => void;
  deshabilitado?: boolean;
}) {
  return (
    <div className="fila">
      <span>{titulo}</span>
      <div className="segmentado" role="group" aria-label={titulo}>
        {opciones.map(([v, n]) => (
          <button
            type="button"
            key={v}
            aria-pressed={v === valor}
            disabled={deshabilitado}
            onClick={() => cambiar(v)}
          >
            {n}
          </button>
        ))}
      </div>
    </div>
  );
}
const rivales = [
  "Sin enemigo",
  "Quieto",
  "Se mueve",
  "Embiste",
  "Flanquea",
  "Tornado",
  "Contragolpe",
  "Woodpecker",
  "Esquivador",
];
const rounds: Opcion[] = [
  ["0,1,3", "Libre"],
  ["1,1,0", "R1 espalda"],
  ["2,1,1", "R2 rival der"],
  ["2,-1,5", "R2 rival izq"],
  ["3,1,2", "R3 enfrentados"],
];
const rumania: Opcion[] = [
  ["4,1,0,0", "Modo 1 de frente"],
  ["4,1,1,0", "Modo 2 de frente"],
  ["4,1,2,90", "Modo 3 de lado"],
  ["4,1,3,180", "Modo 4 de espaldas"],
];
function accion(l: number, r: number) {
  if (l === 0 && r === 0) return "Frena";
  if (l < 0 && r < 0) return "Retrocede";
  if (l > 0 && r < 0) return "Gira a la derecha";
  if (l < 0 && r > 0) return "Gira a la izquierda";
  return l === r
    ? "Avanza"
    : l > r
      ? "Curva a la derecha"
      : "Curva a la izquierda";
}
function Tablero({
  simulador,
  vista,
}: {
  simulador: Simulador;
  vista: string;
}) {
  const canvas = useRef<HTMLCanvasElement>(null),
    canvas3d = useRef<HTMLCanvasElement>(null);
  const escena = useRef<ReturnType<typeof crearEscena> | null>(null);
  const arrastre = useRef<{
    tipo: "robot" | "enemigo" | "girar";
    dx: number;
    dy: number;
  } | null>(null);
  useEffect(() => {
    const dibujo = crearEscena(
      canvas.current!,
      canvas3d.current!,
      simulador.estado,
    );
    escena.current = dibujo;
    let cuadro = 0,
      previo: number | null = null;
    function avanzar(t: number) {
      if (previo !== null)
        simulador.avanzar(Math.min(0.05, (t - previo) / 1000));
      previo = t;
      dibujo.dibujar();
      cuadro = requestAnimationFrame(avanzar);
    }
    cuadro = requestAnimationFrame(avanzar);
    return () => {
      cancelAnimationFrame(cuadro);
      dibujo.destruir();
      escena.current = null;
    };
  }, [simulador]);
  useEffect(() => {
    try {
      escena.current?.vista(vista === "3d");
    } catch (error) {
      simulador.error(error);
    }
  }, [vista, simulador]);
  const mundo = (ev: React.PointerEvent<HTMLCanvasElement>) => {
    const rect = ev.currentTarget.getBoundingClientRect(),
      escala = rect.width / 0.84;
    return [
      (ev.clientX - rect.left - rect.width / 2) / escala,
      -(ev.clientY - rect.top - rect.width / 2) / escala,
    ];
  };
  const soltar = () => {
    arrastre.current = null;
    canvas.current?.classList.remove("arrastrando");
  };
  return (
    <>
      <canvas
        ref={canvas}
        hidden={vista === "3d"}
        aria-label="Dohyo: arrastra el robot y el enemigo para colocarlos"
        onPointerDown={(ev) => {
          if (simulador.estado.masiva) return;
          const s = simulador.estado;
          if (s.fase !== "colocar") simulador.recolocar();
          const [x, y] = mundo(ev),
            r = s.robot,
            hx = r.x + 0.09 * Math.cos(r.th),
            hy = r.y + 0.09 * Math.sin(r.th);
          let tipo: "robot" | "enemigo" | "girar";
          if (Math.hypot(x - hx, y - hy) < 0.025) tipo = "girar";
          else if (Math.hypot(x - r.x, y - r.y) < 0.075) tipo = "robot";
          else if (
            s.modoEnemigo &&
            Math.hypot(x - s.enemigo.x, y - s.enemigo.y) < 0.06
          )
            tipo = "enemigo";
          else return;
          const p = tipo === "enemigo" ? s.enemigo : r;
          arrastre.current = { tipo, dx: p.x - x, dy: p.y - y };
          s.reproducir = false;
          ev.currentTarget.setPointerCapture(ev.pointerId);
          ev.currentTarget.classList.add("arrastrando");
        }}
        onPointerMove={(ev) => {
          const a = arrastre.current;
          if (!a) return;
          const [x, y] = mundo(ev),
            s = simulador.estado;
          if (a.tipo === "girar")
            s.robot.th = Math.atan2(y - s.robot.y, x - s.robot.x);
          else {
            const p: Pose = s[a.tipo],
              max = a.tipo === "robot" ? 0.385 : 0.335,
              px = x + a.dx,
              py = y + a.dy,
              radio = Math.hypot(px, py),
              f = radio > max ? max / radio : 1;
            p.x = px * f;
            p.y = py * f;
          }
          simulador.avisar();
        }}
        onPointerUp={soltar}
        onPointerCancel={soltar}
      />
      <canvas
        ref={canvas3d}
        hidden={vista !== "3d"}
        aria-label="Vista 3D: arrastra para girar la cámara y usa la rueda para acercar"
      />
    </>
  );
}
function App() {
  const [simulador] = useState(() => new Simulador()),
    [vista, setVista] = useState("2d");
  useSyncExternalStore(simulador.suscribir, simulador.instantanea);
  const s = simulador.estado,
    e = s.sim;
  useEffect(() => () => simulador.destruir(), [simulador]);
  const cambiar = (fn: () => void) => {
    simulador.recolocar();
    fn();
    simulador.avisar();
  };
  const elegirRonda = (r: string) => {
    s.ronda = r;
    void simulador.colocar();
  };
  const estado =
    s.fase === "combate"
      ? "En combate"
      : s.fase === "cargando"
        ? "Cargando firmware…"
        : s.cayoEn !== null
          ? `Cayó a los ${s.cayoEn.toFixed(2)} s`
          : s.ganoEn !== null
            ? `¡Ganó! a los ${s.ganoEn.toFixed(2)} s`
            : s.fase === "fin"
              ? "Aguantó los 30 s"
              : "Coloca el robot";
  return (
    <main className="pagina">
      <header>
        <div className="ceja">
          Mini sumo · Leonardo / JSumo XMotion · dohyo Ø 77 cm
        </div>
        <h1>Dohyo Sumo Leonardo</h1>
        <p className="lead">
          Firmware real y física compartida con el simulador de consola.
          Arrastra los robots o su asa de orientación en 2D. En 3D, arrastra
          para girar la cámara y usa la rueda para acercarte.
        </p>
      </header>
      {s.error && (
        <p role="alert" className="error">
          {s.error}
        </p>
      )}
      <div className="mesa">
        <section className="tablero" aria-label="Simulación">
          <Tablero simulador={simulador} vista={vista} />
          <div
            className="estado telemetria"
            aria-live={s.fase === "fin" ? "polite" : "off"}
          >
            <span className={`chip ${s.cayoEn !== null ? "cayo" : ""}`}>
              {estado}
            </span>
            <span>
              Tiempo: <b className="num">{s.t.toFixed(2)} s</b>
            </span>
            <span>
              Motores:{" "}
              <b className="num">
                {e ? `${e.sim_izq()} / ${e.sim_der()}` : "— / —"}
              </b>
            </span>
            <span>
              Acción: <b>{e ? accion(e.sim_izq(), e.sim_der()) : "—"}</b>
            </span>
            <span>
              START: <b>{e ? (e.sim_start() ? "activa" : "CORTADA") : "—"}</b>
            </span>
          </div>
          <p className="ayuda">
            340 g · 400 rpm · ruedas de 3 cm · LiPo 2S. La simulación permite
            explorar hipótesis; los factores físicos requieren calibración con
            el robot real.
          </p>
        </section>
        <aside className="panel">
          <section className="grupo">
            <Selector
              titulo="Vista"
              valor={vista}
              opciones={[
                ["2d", "2D (colocar robots)"],
                ["3d", "3D"],
              ]}
              cambiar={setVista}
            />
            <Selector
              titulo="Firmware"
              valor={s.firmware}
              opciones={[
                ["nuevo", "Nuevo"],
                ["anterior", "Anterior (torneo)"],
              ]}
              deshabilitado={s.masiva}
              cambiar={(v) =>
                cambiar(() => {
                  s.firmware = v as Firmware;
                })
              }
            />
            <Selector
              titulo="Enemigo"
              valor={String(s.modoEnemigo)}
              opciones={rivales.map((n, i) => [String(i), n])}
              deshabilitado={s.masiva}
              cambiar={(v) => {
                s.modoEnemigo = +v;
                void simulador.colocar();
              }}
            />
            <Selector
              titulo="Round (posición y DIP)"
              valor={s.ronda}
              opciones={rounds}
              cambiar={elegirRonda}
              deshabilitado={s.masiva}
            />
            <Selector
              titulo="Rumania (RoboChallenge)"
              valor={s.ronda}
              opciones={rumania}
              cambiar={elegirRonda}
              deshabilitado={s.masiva}
            />
            <p className="ayuda">
              Rumania: modos 1 y 2 de frente, 3 de lado, 4 de espaldas. START
              inicia el combate sin espera de 5 s.
            </p>
            <label className="fila">
              Orientación: {Math.round((s.robot.th * 180) / Math.PI)}°
              <input
                aria-label="Orientación del robot"
                type="range"
                min="-180"
                max="180"
                value={Math.round((s.robot.th * 180) / Math.PI)}
                disabled={s.fase !== "colocar" || s.masiva}
                onChange={(ev) =>
                  cambiar(() => {
                    s.robot.th = (+ev.target.value * Math.PI) / 180;
                    s.reproducir = false;
                  })
                }
              />
            </label>
            <Selector
              titulo="Velocidad"
              valor={String(s.vel)}
              opciones={[
                ["0.25", "¼×"],
                ["0.5", "½×"],
                ["1", "1×"],
              ]}
              cambiar={(v) => {
                s.vel = +v;
                simulador.avisar();
              }}
            />
            <div className="botones">
              <button
                className="boton principal"
                disabled={
                  s.fase === "combate" || s.fase === "cargando" || s.masiva
                }
                onClick={() => void simulador.iniciar()}
              >
                Iniciar combate
              </button>
              <button
                className="boton"
                disabled={s.masiva}
                onClick={() => simulador.recolocar()}
              >
                Volver a colocar
              </button>
              <button
                className="boton"
                disabled={s.masiva}
                onClick={() => void simulador.colocar(true)}
              >
                Posición al azar
              </button>
            </div>
          </section>
          <section className="grupo">
            <details open>
              <summary>Robot y factores externos</summary>
              {factores.map((f) => (
                <Selector
                  key={f.clave}
                  titulo={f.titulo}
                  valor={s.params[f.clave]}
                  opciones={f.opciones.map((o) => [o.valor, o.nombre])}
                  deshabilitado={s.masiva}
                  cambiar={(v) =>
                    cambiar(() => {
                      s.params[f.clave] = v;
                    })
                  }
                />
              ))}
            </details>
          </section>
          <section className="grupo">
            <h2>Prueba de 200 combates</h2>
            <button
              className="boton"
              disabled={s.masiva}
              onClick={() => void simulador.probar()}
            >
              {s.masiva
                ? `Probando… ${s.progreso}/200`
                : "Probar posiciones al azar"}
            </button>
            <div role="status">
              {s.progreso > 0 &&
                `${s.progreso} combates · ${s.victorias} victorias · ${s.caidas.length} caídas`}
            </div>
            <div className="semillas">
              {s.caidas.slice(0, 24).map((c) => (
                <button
                  key={c.semilla}
                  disabled={s.masiva}
                  onClick={() => simulador.repetir(c)}
                >
                  cae a {c.t.toFixed(1)} s · {c.semilla}
                </button>
              ))}
            </div>
          </section>
        </aside>
      </div>
    </main>
  );
}
createRoot(document.getElementById("root")!).render(<App />);
