// Contrato del adaptador C++ simulacion/web/sim_web.cpp.
export type Firmware = "nuevo" | "anterior";
export interface SimWasm {
  __wasm_call_ctors(): void;
  sim_reiniciar(semilla: number): void;
  sim_fijar(id: number, valor: number): void;
  sim_round(salida: number, lado: number, dip: number): void;
  sim_angulo_rumania(grados: number): void;
  sim_robot(x: number, y: number, th: number): void;
  sim_enemigo(modo: number, x: number, y: number, th: number): void;
  sim_aleatorio(modo: number): void;
  sim_setup(): void;
  sim_avanzar(t: number): number;
  sim_tiempo(): number;
  sim_x(): number;
  sim_y(): number;
  sim_th(): number;
  sim_ex(): number;
  sim_ey(): number;
  sim_eth(): number;
  sim_izq(): number;
  sim_der(): number;
  sim_start(): number;
  sim_linea(): number;
  sim_vista(i: number): number;
}
const modulos = new Map<Firmware, Promise<WebAssembly.Module>>();
export async function nuevaInstancia(firmware: Firmware): Promise<SimWasm> {
  let modulo = modulos.get(firmware);
  if (!modulo) {
    modulo = fetch(`${import.meta.env.BASE_URL}sim-${firmware}.wasm`).then(
      async (r) => {
        if (!r.ok)
          throw new Error(
            `No se pudo cargar el firmware ${firmware}: HTTP ${r.status}`,
          );
        return WebAssembly.compile(await r.arrayBuffer());
      },
    );
    modulos.set(firmware, modulo);
    modulo.catch(() => modulos.delete(firmware));
  }
  const instancia = await WebAssembly.instantiate(await modulo, {
    env: { sin: Math.sin, cos: Math.cos, atan2: Math.atan2, log: Math.log },
  });
  const api = instancia.exports as unknown as SimWasm;
  api.__wasm_call_ctors();
  return api;
}
