import { nuevaInstancia, type Firmware, type SimWasm } from "./wasm";
export interface Pose {
  x: number;
  y: number;
  th: number;
}
export interface Estado {
  firmware: Firmware;
  modoEnemigo: number;
  vel: number;
  ronda: string;
  robot: Pose;
  enemigo: Pose;
  params: Record<string, string>;
  fase: "colocar" | "cargando" | "combate" | "fin";
  semilla: number;
  sim: SimWasm | null;
  t: number;
  rastro: number[][];
  cayoEn: number | null;
  ganoEn: number | null;
  reproducir: boolean;
  error: string;
  masiva: boolean;
  progreso: number;
  victorias: number;
  caidas: Caida[];
}
interface Caida {
  semilla: number;
  t: number;
  robot: Pose;
  enemigo: Pose;
}
export class Simulador {
  readonly estado: Estado = {
    firmware: "nuevo",
    modoEnemigo: 1,
    vel: 1,
    ronda: "0,1,3",
    robot: { x: 0, y: -0.15, th: Math.PI / 2 },
    enemigo: { x: 0, y: 0.15, th: -Math.PI / 2 },
    params: {
      bateria: "1.23",
      motores: "0.15,0.5",
      ruedas: "0.03",
      piso: "900,100,1000",
      pared: "0",
      rival: "0.8,0.3,0.6",
      dohyo: "0.9,1",
      llantas: "0,0",
      cortes: "0",
      sensores: "0,0",
      retardo: "0",
    },
    fase: "colocar",
    semilla: 1,
    sim: null,
    t: 0,
    rastro: [],
    cayoEn: null,
    ganoEn: null,
    reproducir: false,
    error: "",
    masiva: false,
    progreso: 0,
    victorias: 0,
    caidas: [],
  };
  private revision = 0;
  private operacion = 0;
  private oyentes = new Set<() => void>();
  readonly suscribir = (fn: () => void) => {
    this.oyentes.add(fn);
    return () => {
      this.oyentes.delete(fn);
    };
  };
  readonly instantanea = () => this.revision;
  avisar() {
    this.revision++;
    this.oyentes.forEach((fn) => fn());
  }
  error(error: unknown) {
    this.estado.error = error instanceof Error ? error.message : String(error);
    this.estado.fase = "colocar";
    this.avisar();
  }
  recolocar() {
    this.operacion++;
    Object.assign(this.estado, {
      fase: "colocar",
      sim: null,
      t: 0,
      cayoEn: null,
      ganoEn: null,
      rastro: [],
      error: "",
    });
    this.avisar();
  }
  private configurar(e: SimWasm) {
    const p = this.estado.params;
    const [fc, fg] = p.motores.split(",").map(Number),
      [n, b, f] = p.piso.split(",").map(Number);
    const [agarre, masa, vel] = p.rival.split(",").map(Number),
      [mu, polvo] = p.dohyo.split(",").map(Number);
    const [izq, der] = p.llantas.split(",").map(Number),
      [reflejos, cegados] = p.sensores.split(",").map(Number);
    const pares = [
      [1, +p.ruedas],
      [3, +p.bateria],
      [4, fc],
      [5, fg],
      [6, n],
      [7, b],
      [8, f],
      [9, +p.pared],
      [10, mu],
      [17, agarre * polvo],
      [18, masa],
      [19, vel],
      [15, izq],
      [16, der],
      [14, +p.cortes],
      [12, reflejos],
      [13, cegados],
      [20, +p.retardo],
    ];
    pares.forEach(([id, v]) => e.sim_fijar(id, v));
    const [salida, lado, dip, angulo = 0] = this.estado.ronda
      .split(",")
      .map(Number);
    e.sim_round(salida, lado, dip);
    e.sim_angulo_rumania(angulo);
  }
  private poses(e: SimWasm) {
    return {
      robot: { x: e.sim_x(), y: e.sim_y(), th: e.sim_th() },
      enemigo: { x: e.sim_ex(), y: e.sim_ey(), th: e.sim_eth() },
    };
  }
  async colocar(azar = false) {
    this.recolocar();
    if (this.estado.ronda.startsWith("0,") && !azar) {
      this.estado.reproducir = false;
      return;
    }
    const turno = this.operacion;
    this.estado.fase = "cargando";
    this.avisar();
    try {
      const e = await nuevaInstancia(this.estado.firmware);
      if (turno !== this.operacion) return;
      this.estado.semilla = 1 + Math.floor(Math.random() * 1e6);
      e.sim_reiniciar(this.estado.semilla);
      this.configurar(e);
      e.sim_fijar(11, 0.34);
      e.sim_aleatorio(this.estado.modoEnemigo);
      Object.assign(this.estado, this.poses(e), {
        reproducir: true,
        fase: "colocar",
      });
      this.avisar();
    } catch (error) {
      if (turno === this.operacion) this.error(error);
    }
  }
  async iniciar() {
    this.recolocar();
    const turno = this.operacion,
      s = this.estado;
    s.fase = "cargando";
    this.avisar();
    try {
      const e = await nuevaInstancia(s.firmware);
      if (turno !== this.operacion) return;
      e.sim_reiniciar(s.semilla);
      this.configurar(e);
      if (s.reproducir) {
        e.sim_fijar(11, 0.34);
        e.sim_aleatorio(s.modoEnemigo);
      } else {
        e.sim_robot(s.robot.x, s.robot.y, s.robot.th);
        e.sim_enemigo(s.modoEnemigo, s.enemigo.x, s.enemigo.y, s.enemigo.th);
      }
      e.sim_setup();
      s.sim = e;
      s.fase = "combate";
      s.rastro = [[s.robot.x, s.robot.y]];
      this.avisar();
    } catch (error) {
      if (turno === this.operacion) this.error(error);
    }
  }
  avanzar(dt: number) {
    const s = this.estado,
      e = s.sim;
    if (s.fase !== "combate" || !e) return;
    s.t = Math.min(30, s.t + dt * s.vel);
    const final = e.sim_avanzar(s.t);
    s.rastro.push([e.sim_x(), e.sim_y()]);
    if (s.rastro.length > 400) s.rastro.shift();
    if (final === 1) {
      s.cayoEn = e.sim_tiempo();
      s.fase = "fin";
    } else if (final === 2) {
      s.ganoEn = e.sim_tiempo();
      s.fase = "fin";
    } else if (s.t >= 30) s.fase = "fin";
    this.avisar();
  }
  async probar() {
    this.recolocar();
    const turno = this.operacion,
      s = this.estado;
    s.masiva = true;
    s.progreso = 0;
    s.victorias = 0;
    s.caidas = [];
    this.avisar();
    const base = 1 + Math.floor(Math.random() * 1e6);
    try {
      for (let i = 0; i < 200; i++) {
        const e = await nuevaInstancia(s.firmware);
        if (turno !== this.operacion) return;
        const semilla = base + i;
        e.sim_reiniciar(semilla);
        this.configurar(e);
        e.sim_fijar(11, 0.34);
        e.sim_aleatorio(s.modoEnemigo);
        const poses = this.poses(e);
        e.sim_setup();
        const final = e.sim_avanzar(30);
        if (final === 1)
          s.caidas.push({ semilla, t: e.sim_tiempo(), ...poses });
        else if (final === 2) s.victorias++;
        s.progreso = i + 1;
        if (i % 10 === 9) {
          this.avisar();
          await new Promise((r) => setTimeout(r, 0));
        }
      }
    } catch (error) {
      if (turno === this.operacion) this.error(error);
    } finally {
      s.masiva = false;
      this.avisar();
    }
  }
  repetir(caida: Caida) {
    this.recolocar();
    Object.assign(this.estado, {
      semilla: caida.semilla,
      robot: { ...caida.robot },
      enemigo: { ...caida.enemigo },
      reproducir: true,
    });
    this.avisar();
  }
  destruir() {
    this.operacion++;
    this.oyentes.clear();
  }
}
