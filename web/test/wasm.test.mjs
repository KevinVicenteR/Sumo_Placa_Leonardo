import test from "node:test";
import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";

const entorno = {
  env: { sin: Math.sin, cos: Math.cos, atan2: Math.atan2, log: Math.log },
};
async function instancia(nombre) {
  const bytes = await readFile(
    new URL(`../public/sim-${nombre}.wasm`, import.meta.url),
  );
  const { instance } = await WebAssembly.instantiate(bytes, entorno);
  const e = instance.exports;
  e.__wasm_call_ctors();
  return e;
}
function colocar(e, modo, rival, perturbado = false) {
  e.sim_reiniciar(424242);
  e.sim_round(4, 1, modo - 1);
  e.sim_angulo_rumania([0, 0, 90, 180][modo - 1]);
  e.sim_fijar(11, 0.34);
  if (perturbado) {
    for (const [id, v] of [
      [3, 1.12],
      [9, 0.5],
      [10, 0.7],
      [12, 1],
      [13, 0.2],
      [14, 0.3],
      [15, 0.5],
      [16, 0.3],
      [20, 30],
    ])
      e.sim_fijar(id, v);
  }
  e.sim_aleatorio(rival);
  e.sim_setup();
}
function lectura(e) {
  return [
    e.sim_tiempo(),
    e.sim_x(),
    e.sim_y(),
    e.sim_th(),
    e.sim_ex(),
    e.sim_ey(),
    e.sim_izq(),
    e.sim_der(),
    e.sim_start(),
  ];
}
for (const firmware of ["nuevo", "anterior"]) {
  test(`${firmware}: instancias aisladas y repetición con la misma semilla`, async () => {
    const a = await instancia(firmware),
      b = await instancia(firmware);
    colocar(a, 4, 6);
    colocar(b, 4, 6);
    a.sim_avanzar(0.8);
    assert.equal(
      b.sim_tiempo(),
      0,
      "la memoria de una instancia no debe afectar a otra",
    );
    b.sim_avanzar(0.8);
    assert.deepEqual(lectura(a), lectura(b));
  });
  test(`${firmware}: modos 1–4, nueve rivales y factores externos`, async () => {
    for (let modo = 1; modo <= 4; modo++)
      for (let rival = 0; rival <= 8; rival++) {
        const e = await instancia(firmware);
        colocar(e, modo, rival, true);
        assert.ok([0, 1, 2].includes(e.sim_avanzar(2)));
        assert.ok(
          lectura(e).every(Number.isFinite),
          `estado inválido: modo ${modo}, rival ${rival}`,
        );
        for (let i = 0; i < 5; i++) assert.ok([0, 1].includes(e.sim_vista(i)));
      }
  });
}
test("Rumania actual: START produce movimiento antes de 5 segundos", async () => {
  for (let modo = 1; modo <= 4; modo++) {
    const e = await instancia("nuevo");
    colocar(e, modo, 1);
    let seMovio = false;
    for (let t = 0.05; t <= 0.8; t += 0.05) {
      e.sim_avanzar(t);
      seMovio ||= e.sim_izq() !== 0 || e.sim_der() !== 0;
    }
    assert.ok(seMovio, `el modo ${modo} no debe esperar 5 s`);
  }
});
