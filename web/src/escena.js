import * as THREE from "three";

// Adaptador gráfico conservado: solo lee el estado; no decide ni simula.
export function crearEscena(canvas, canvas3d, estado) {
  const ctx = canvas.getContext("2d");
  const R = 0.385,
    BORDE = 0.025,
    LARGO = 0.1,
    SX = 0.045,
    SY = 0.04;
  const css = (n) =>
    getComputedStyle(document.documentElement).getPropertyValue(n).trim();
  const eventos = new AbortController();
  const escuchar = (el, tipo, fn, opciones = {}) =>
    el.addEventListener(tipo, fn, { ...opciones, signal: eventos.signal });
  function aMundo(o, rx, ry) {
    const c = Math.cos(o.th),
      s = Math.sin(o.th);
    return [o.x + c * rx - s * ry, o.y + s * rx + c * ry];
  }
  function cuerpoDentro(o) {
    return [
      [-1, -1],
      [-1, 1],
      [1, -1],
      [1, 1],
    ].every(
      ([a, b]) =>
        Math.hypot(...aMundo(o, (a * LARGO) / 2, (b * LARGO) / 2)) <= R,
    );
  }
  function sensorSobreLinea(o) {
    return [SY, -SY].some((sy) => {
      const r = Math.hypot(...aMundo(o, SX, sy));
      return r > R - BORDE - 0.003 && r <= R + 0.003;
    });
  }

  // Rectángulo con esquinas redondeadas (en metros, marco del robot: +x = frente)
  function caja(s, x, y, w, h, radio, relleno, borde) {
    ctx.beginPath();
    ctx.roundRect(x * s, y * s, w * s, h * s, radio * s);
    if (relleno) {
      ctx.fillStyle = relleno;
      ctx.fill();
    }
    if (borde) {
      ctx.strokeStyle = borde;
      ctx.lineWidth = 1;
      ctx.stroke();
    }
  }

  // Tu sumo visto desde arriba, como en el video: chasis negro, pala de metal
  // delante, placa XMotion blanca, ruedas JSumo blancas atrás, batería y cables
  function dibujarMiRobot(s, linea, acento) {
    // Chasis negro
    caja(s, -0.05, -0.05, 0.09, 0.1, 0.004, "#17181b", "#2c2e33");
    // Ruedas JSumo blancas y anchas (3 cm de diámetro) en las esquinas traseras
    for (const lado of [-1, 1]) {
      const y = lado > 0 ? 0.025 : -0.051;
      caja(s, -0.051, y, 0.034, 0.026, 0.005, "#efeee9", "#a9a8a1");
      ctx.strokeStyle = "#cfcec7";
      ctx.lineWidth = 1;
      for (let x = -0.047; x < -0.019; x += 0.004) {
        ctx.beginPath();
        ctx.moveTo(x * s, (y + 0.003) * s);
        ctx.lineTo(x * s, (y + 0.023) * s);
        ctx.stroke();
      }
    }
    // Batería LiPo (termorretráctil negro) atrás
    caja(s, -0.046, -0.021, 0.028, 0.042, 0.003, "#0b0b0c", "#3a3c42");
    // Placa XMotion blanca con su microcontrolador y LEDs
    caja(s, -0.014, -0.032, 0.05, 0.064, 0.003, "#f3f3ef", "#c8c8c2");
    caja(s, 0.002, -0.007, 0.014, 0.014, 0.001, "#1b1c20");
    for (const [x, y, c] of [
      [-0.008, 0.024, "#ff3b30"],
      [-0.008, 0.016, "#2f7bff"],
      [0.028, -0.024, "#ff3b30"],
      [0.028, 0.024, "#34c759"],
    ]) {
      ctx.beginPath();
      ctx.arc(x * s, y * s, 0.0022 * s, 0, 2 * Math.PI);
      ctx.fillStyle = c;
      ctx.fill();
    }
    // Interruptores DIP
    caja(s, 0.018, 0.012, 0.012, 0.016, 0.001, "#d23b3b");
    // Cables rojos y amarillos
    ctx.lineWidth = Math.max(1, 0.0016 * s);
    ctx.lineCap = "round";
    for (const [c, y0, y1] of [
      ["#d6262b", 0.026, 0.012],
      ["#e3b10f", -0.026, -0.012],
      ["#d6262b", -0.018, 0.004],
    ]) {
      ctx.strokeStyle = c;
      ctx.beginPath();
      ctx.moveTo(-0.032 * s, y0 * s);
      ctx.bezierCurveTo(
        -0.02 * s,
        (y0 + 0.012) * s,
        -0.004 * s,
        (y1 - 0.01) * s,
        0.008 * s,
        y1 * s,
      );
      ctx.stroke();
    }
    // Pala de metal delante (biselada)
    const g = ctx.createLinearGradient(0.04 * s, 0, 0.054 * s, 0);
    g.addColorStop(0, "#9aa1a9");
    g.addColorStop(0.55, "#dfe3e7");
    g.addColorStop(1, "#7d848c");
    ctx.beginPath();
    ctx.moveTo(0.038 * s, -0.05 * s);
    ctx.lineTo(0.05 * s, -0.052 * s);
    ctx.lineTo(0.05 * s, 0.052 * s);
    ctx.lineTo(0.038 * s, 0.05 * s);
    ctx.closePath();
    ctx.fillStyle = g;
    ctx.fill();
    ctx.strokeStyle = "#6c737b";
    ctx.lineWidth = 1;
    ctx.stroke();
    // Sensores de piso (bajo la pala): rojos cuando ven la línea
    for (const sy of [-SY, SY]) {
      ctx.beginPath();
      ctx.arc(SX * s, sy * s, 0.0045 * s, 0, 2 * Math.PI);
      ctx.fillStyle = linea ? "#ff3b30" : "#2a2c31";
      ctx.fill();
      ctx.strokeStyle = "#ffffff";
      ctx.lineWidth = 1;
      ctx.stroke();
    }
    // Contorno del color del firmware (azul el nuevo, naranja el del torneo)
    ctx.strokeStyle = acento;
    ctx.lineWidth = 2;
    ctx.strokeRect(-0.052 * s, -0.054 * s, 0.104 * s, 0.108 * s);
  }

  // El rival del video: chasis rojo, ruedas naranjas, pala negra y placa azul
  function dibujarRival(s) {
    for (const lado of [-1, 1]) {
      const y = lado > 0 ? 0.03 : -0.052;
      caja(s, -0.048, y, 0.032, 0.022, 0.005, "#ff6a2b", "#c94c18");
    }
    caja(s, -0.05, -0.044, 0.09, 0.088, 0.005, "#b3201e", "#7d1513");
    caja(s, -0.03, -0.022, 0.04, 0.03, 0.002, "#1f5fa8", "#123d6e");
    ctx.beginPath();
    ctx.arc(-0.004 * s, 0.018 * s, 0.006 * s, 0, 2 * Math.PI);
    ctx.fillStyle = "rgba(255,60,60,.85)";
    ctx.fill();
    const g = ctx.createLinearGradient(0.04 * s, 0, 0.054 * s, 0);
    g.addColorStop(0, "#3a3c41");
    g.addColorStop(0.6, "#5b5e65");
    g.addColorStop(1, "#202226");
    ctx.beginPath();
    ctx.moveTo(0.038 * s, -0.05 * s);
    ctx.lineTo(0.05 * s, -0.052 * s);
    ctx.lineTo(0.05 * s, 0.052 * s);
    ctx.lineTo(0.038 * s, 0.05 * s);
    ctx.closePath();
    ctx.fillStyle = g;
    ctx.fill();
  }

  // Los 5 sensores de enemigo como rayos de 40 cm: rojo el que ve al rival
  const RAYOS = [
    [0.05, 0, 0],
    [0.05, 0.03, Math.PI / 4],
    [0.05, -0.03, -Math.PI / 4],
    [0, 0.05, Math.PI / 2],
    [0, -0.05, -Math.PI / 2],
  ];
  function dibujarRayos(s, e) {
    RAYOS.forEach(([x, y, a], i) => {
      const ve = e ? e.sim_vista(i) === 1 : false;
      ctx.strokeStyle = ve ? "rgba(255,59,48,.85)" : "rgba(255,255,255,.13)";
      ctx.lineWidth = ve ? 2 : 1;
      ctx.setLineDash(ve ? [] : [3, 4]);
      ctx.beginPath();
      ctx.moveTo(x * s, -y * s);
      ctx.lineTo((x + 0.4 * Math.cos(a)) * s, -(y + 0.4 * Math.sin(a)) * s);
      ctx.stroke();
    });
    ctx.setLineDash([]);
  }

  // ---------------------------------------------------------------- vista 3D
  // Escena 3D con three.js. Usa el mismo estado que la vista 2D; solo cambia el
  // dibujo. Ejes: x de la simulación -> X, y -> -Z, altura -> Y (metros).
  const vista3d = {
    activa: false,
    listo: false,
    radio: 1.35,
    theta: 0.9,
    phi: 0.85,
    giroRuedas: 0,
  };

  function material3d(color, extra = {}) {
    return new THREE.MeshStandardMaterial({
      color,
      roughness: 0.6,
      metalness: 0,
      ...extra,
    });
  }
  function bloque3d(w, h, d, mat, x, y, z) {
    const m = new THREE.Mesh(new THREE.BoxGeometry(w, h, d), mat);
    m.position.set(x, y, z);
    m.castShadow = true;
    m.receiveShadow = true;
    return m;
  }
  // Rueda de 3 cm de diámetro con su eje a lo ancho del robot
  function rueda3d(color, x, z) {
    const g = new THREE.CylinderGeometry(0.015, 0.015, 0.024, 28);
    g.rotateX(Math.PI / 2);
    const m = new THREE.Mesh(g, material3d(color, { roughness: 0.9 }));
    m.position.set(x, 0.015, z);
    m.castShadow = true;
    return m;
  }
  // Pala biselada delante: perfil triangular extruido a lo ancho
  function pala3d(color, metal) {
    const perfil = new THREE.Shape();
    perfil.moveTo(0.034, 0.004);
    perfil.lineTo(0.053, 0.001);
    perfil.lineTo(0.034, 0.024);
    perfil.closePath();
    const g = new THREE.ExtrudeGeometry(perfil, {
      depth: 0.104,
      bevelEnabled: false,
    });
    g.translate(0, 0, -0.052);
    const m = new THREE.Mesh(
      g,
      material3d(color, { metalness: metal, roughness: metal ? 0.28 : 0.6 }),
    );
    m.castShadow = true;
    return m;
  }
  // Rayo de un sensor de enemigo (40 cm)
  function rayo3d(x, y, a) {
    const g = new THREE.BufferGeometry().setFromPoints([
      new THREE.Vector3(x, 0.02, -y),
      new THREE.Vector3(x + 0.4 * Math.cos(a), 0.02, -(y + 0.4 * Math.sin(a))),
    ]);
    return new THREE.Line(
      g,
      new THREE.LineBasicMaterial({
        color: 0xffffff,
        transparent: true,
        opacity: 0.18,
      }),
    );
  }

  // Tu sumo: chasis negro, pala de metal, placa XMotion blanca, batería y ruedas blancas
  function crearMiRobot3d() {
    const g = new THREE.Group();
    g.add(bloque3d(0.086, 0.02, 0.1, material3d(0x17181b), -0.006, 0.014, 0));
    const ruedas = [
      rueda3d(0xefeee9, -0.033, 0.038),
      rueda3d(0xefeee9, -0.033, -0.038),
    ];
    ruedas.forEach((w) => g.add(w));
    g.add(
      bloque3d(0.028, 0.016, 0.042, material3d(0x0b0b0c), -0.032, 0.032, 0),
    );
    g.add(bloque3d(0.05, 0.003, 0.064, material3d(0xf3f3ef), 0.011, 0.0255, 0));
    g.add(
      bloque3d(0.014, 0.003, 0.014, material3d(0x1b1c20), 0.009, 0.0285, 0),
    );
    for (const [x, z, c] of [
      [-0.008, -0.024, 0xff3b30],
      [-0.008, -0.016, 0x2f7bff],
      [0.028, 0.024, 0xff3b30],
      [0.028, -0.024, 0x34c759],
    ]) {
      g.add(
        bloque3d(
          0.004,
          0.003,
          0.004,
          material3d(c, { emissive: c, emissiveIntensity: 1.5 }),
          x,
          0.0285,
          z,
        ),
      );
    }
    g.add(
      bloque3d(0.012, 0.004, 0.016, material3d(0xd23b3b), 0.024, 0.0285, -0.02),
    );
    g.add(pala3d(0xd9dde2, 0.85));
    // Contorno del color del firmware (azul el nuevo, naranja el del torneo)
    const contorno = new THREE.LineSegments(
      new THREE.EdgesGeometry(new THREE.BoxGeometry(0.104, 0.026, 0.104)),
      new THREE.LineBasicMaterial({ color: 0x5fa8e6 }),
    );
    contorno.position.set(0, 0.013, 0);
    g.add(contorno);
    // Sensores de piso bajo la pala (se encienden en rojo sobre la línea)
    const piso = [SY, -SY].map((sy) => {
      const m = new THREE.Mesh(
        new THREE.SphereGeometry(0.0045, 14, 10),
        material3d(0x2a2c31),
      );
      m.position.set(SX, 0.006, -sy);
      g.add(m);
      return m;
    });
    const rayos = RAYOS.map(([x, y, a]) => {
      const l = rayo3d(x, y, a);
      g.add(l);
      return l;
    });
    g.userData = { ruedas, piso, rayos, contorno };
    return g;
  }

  // El rival del video: chasis rojo, ruedas naranjas, pala negra y placa azul
  function crearRival3d() {
    const g = new THREE.Group();
    g.add(
      bloque3d(0.086, 0.022, 0.096, material3d(0xb3201e), -0.006, 0.015, 0),
    );
    const ruedas = [
      rueda3d(0xff6a2b, -0.032, 0.038),
      rueda3d(0xff6a2b, -0.032, -0.038),
    ];
    ruedas.forEach((w) => g.add(w));
    g.add(
      bloque3d(0.04, 0.004, 0.03, material3d(0x1f5fa8), -0.01, 0.028, 0.007),
    );
    g.add(
      bloque3d(
        0.006,
        0.004,
        0.006,
        material3d(0xff3030, { emissive: 0xff3030, emissiveIntensity: 2 }),
        -0.004,
        0.03,
        -0.018,
      ),
    );
    g.add(pala3d(0x3a3c41, 0.4));
    g.userData = { ruedas };
    return g;
  }

  function iniciar3D() {
    if (vista3d.listo) return true;
    if (typeof THREE === "undefined") return false;
    const lienzo = canvas3d;
    const renderer = new THREE.WebGLRenderer({
      canvas: lienzo,
      antialias: true,
    });
    renderer.shadowMap.enabled = true;
    renderer.shadowMap.type = THREE.PCFSoftShadowMap;
    const escena = new THREE.Scene();
    const camara = new THREE.PerspectiveCamera(38, 1, 0.01, 20);

    // Luces
    escena.add(new THREE.HemisphereLight(0xffffff, 0x3a3a3a, 0.75));
    const sol = new THREE.DirectionalLight(0xffffff, 0.9);
    sol.position.set(0.5, 1.4, 0.6);
    sol.castShadow = true;
    sol.shadow.mapSize.set(2048, 2048);
    Object.assign(sol.shadow.camera, {
      left: -0.6,
      right: 0.6,
      top: 0.6,
      bottom: -0.6,
      near: 0.1,
      far: 4,
    });
    escena.add(sol);

    // Suelo y dohyo (cilindro negro de 5 cm con borde blanco)
    const suelo = new THREE.Mesh(
      new THREE.PlaneGeometry(4, 4),
      material3d(0x8f887c, { roughness: 0.95 }),
    );
    suelo.rotation.x = -Math.PI / 2;
    suelo.position.y = -0.05;
    suelo.receiveShadow = true;
    escena.add(suelo);
    const dohyo = new THREE.Mesh(
      new THREE.CylinderGeometry(R, R, 0.05, 96),
      material3d(0x101113, { roughness: 0.85 }),
    );
    dohyo.position.y = -0.025;
    dohyo.receiveShadow = true;
    dohyo.castShadow = true;
    escena.add(dohyo);
    const borde = new THREE.Mesh(
      new THREE.RingGeometry(R - BORDE, R, 96),
      material3d(0xf7f7f5, { roughness: 0.7 }),
    );
    borde.rotation.x = -Math.PI / 2;
    borde.position.y = 0.0005;
    borde.receiveShadow = true;
    escena.add(borde);
    for (const x of [-0.045, 0.045]) {
      const linea = new THREE.Mesh(
        new THREE.PlaneGeometry(0.01, 0.1),
        material3d(0x6b4a2b),
      );
      linea.rotation.x = -Math.PI / 2;
      linea.position.set(x, 0.0006, 0);
      escena.add(linea);
    }

    const robot = crearMiRobot3d(),
      rival = crearRival3d();
    escena.add(robot);
    escena.add(rival);
    const rastro = new THREE.Line(
      new THREE.BufferGeometry(),
      new THREE.LineBasicMaterial({ color: 0x5fa8e6 }),
    );
    escena.add(rastro);

    Object.assign(vista3d, {
      renderer,
      escena,
      camara,
      robot,
      rival,
      rastro,
      listo: true,
    });

    // Cámara orbital: arrastrar gira, rueda o pellizco acerca
    let previo = null;
    escuchar(lienzo, "pointerdown", (ev) => {
      previo = [ev.clientX, ev.clientY];
      lienzo.setPointerCapture(ev.pointerId);
    });
    escuchar(lienzo, "pointermove", (ev) => {
      if (!previo) return;
      vista3d.theta -= (ev.clientX - previo[0]) * 0.01;
      vista3d.phi = Math.min(
        1.45,
        Math.max(0.12, vista3d.phi - (ev.clientY - previo[1]) * 0.01),
      );
      previo = [ev.clientX, ev.clientY];
      dibujar();
    });
    escuchar(lienzo, "pointerup", () => {
      previo = null;
    });
    escuchar(lienzo, "pointercancel", () => {
      previo = null;
    });
    escuchar(
      lienzo,
      "wheel",
      (ev) => {
        ev.preventDefault();
        vista3d.radio = Math.min(
          2.5,
          Math.max(0.3, vista3d.radio * (ev.deltaY > 0 ? 1.1 : 0.9)),
        );
        dibujar();
      },
      { passive: false },
    );
    return true;
  }

  // Coloca un robot 3D: si su centro sale del dohyo, cae al suelo
  function colocar3d(obj, o) {
    const fuera = Math.hypot(o.x, o.y) > R;
    obj.position.set(o.x, fuera ? -0.05 : 0, -o.y);
    obj.rotation.set(fuera ? 0.35 : 0, o.th || 0, 0);
  }

  function dibujar3D(robot, enemigo, linea, acento) {
    const v = vista3d,
      lienzo = canvas3d;
    const w = lienzo.clientWidth;
    if (!w) return;
    const dpr = window.devicePixelRatio || 1;
    if (lienzo.width !== Math.round(w * dpr)) {
      v.renderer.setPixelRatio(dpr);
      v.renderer.setSize(w, w, false);
    }
    v.escena.background = new THREE.Color(css("--surface"));

    colocar3d(v.robot, robot);
    v.rival.visible = estado.modoEnemigo !== 0;
    if (v.rival.visible) colocar3d(v.rival, enemigo);

    // Ruedas girando según la orden de los motores
    if (estado.sim && estado.fase === "combate") {
      const e = estado.sim;
      v.robot.userData.ruedas[0].rotation.z -= (e.sim_izq() / 255) * 0.5;
      v.robot.userData.ruedas[1].rotation.z -= (e.sim_der() / 255) * 0.5;
      for (const r of v.rival.userData.ruedas) r.rotation.z -= 0.3;
    }
    // Sensores de piso, rayos de los sensores de enemigo y color del firmware
    for (const m of v.robot.userData.piso)
      m.material.emissive.setHex(linea ? 0xff2a1f : 0x000000);
    v.robot.userData.rayos.forEach((l, i) => {
      const ve = estado.sim ? estado.sim.sim_vista(i) === 1 : false;
      l.visible = estado.modoEnemigo !== 0;
      l.material.color.setHex(ve ? 0xff3b30 : 0xffffff);
      l.material.opacity = ve ? 0.9 : 0.18;
    });
    v.robot.userData.contorno.material.color.set(acento);
    v.rastro.material.color.set(acento);
    v.rastro.geometry.setFromPoints(
      estado.rastro.map(([x, y]) => new THREE.Vector3(x, 0.002, -y)),
    );

    // Cámara
    const c = v.camara;
    c.position.set(
      v.radio * Math.sin(v.phi) * Math.cos(v.theta),
      v.radio * Math.cos(v.phi),
      v.radio * Math.sin(v.phi) * Math.sin(v.theta),
    );
    c.lookAt(0, 0, 0);
    v.renderer.render(v.escena, c);
  }

  function escala() {
    const w = canvas.clientWidth;
    return { w, s: w / 0.84, c: w / 2 };
  }
  function dibujar() {
    const dpr = window.devicePixelRatio || 1;
    const { w, s, c } = escala();
    if (canvas.width !== Math.round(w * dpr)) {
      canvas.width = Math.round(w * dpr);
      canvas.height = Math.round(w * dpr);
    }
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    const X = (x) => c + x * s,
      Y = (y) => c - y * s;

    ctx.fillStyle = css("--surface");
    ctx.fillRect(0, 0, w, w);
    if (Number(estado.params.pared) > 0) {
      ctx.setLineDash([4, 6]);
      ctx.strokeStyle = css("--muted");
      ctx.lineWidth = 1.5;
      ctx.beginPath();
      ctx.arc(c, c, Number(estado.params.pared) * s, 0, 2 * Math.PI);
      ctx.stroke();
      ctx.setLineDash([]);
    }
    ctx.beginPath();
    ctx.arc(c, c, R * s, 0, 2 * Math.PI);
    ctx.fillStyle = "#f7f7f5";
    ctx.fill();
    ctx.lineWidth = 1;
    ctx.strokeStyle = css("--rule");
    ctx.stroke();
    ctx.beginPath();
    ctx.arc(c, c, (R - BORDE) * s, 0, 2 * Math.PI);
    ctx.fillStyle = "#101113";
    ctx.fill();
    ctx.fillStyle = "#6b4a2b"; // shikiri-sen
    ctx.fillRect(X(-0.05), Y(0.05), 0.01 * s, 0.1 * s);
    ctx.fillRect(X(0.04), Y(0.05), 0.01 * s, 0.1 * s);

    const color = css(estado.firmware === "nuevo" ? "--robot" : "--antes");
    let robot = estado.robot,
      enemigo = estado.enemigo,
      lin = false;
    if (estado.sim) {
      const e = estado.sim;
      robot = { x: e.sim_x(), y: e.sim_y(), th: e.sim_th() };
      enemigo = { x: e.sim_ex(), y: e.sim_ey(), th: e.sim_eth() };
      lin = e.sim_linea() === 1;
      ctx.lineWidth = 2;
      ctx.lineCap = "round";
      ctx.strokeStyle = color;
      const r = estado.rastro;
      for (let k = 1; k < r.length; k++) {
        ctx.globalAlpha = 0.08 + (0.6 * k) / r.length;
        ctx.beginPath();
        ctx.moveTo(X(r[k - 1][0]), Y(r[k - 1][1]));
        ctx.lineTo(X(r[k][0]), Y(r[k][1]));
        ctx.stroke();
      }
      ctx.globalAlpha = 1;
    }

    if (estado.modoEnemigo !== 0) {
      ctx.save();
      ctx.translate(X(enemigo.x), Y(enemigo.y));
      ctx.rotate(-(enemigo.th || 0));
      dibujarRival(s);
      ctx.restore();
    }

    const caido = estado.cayoEn !== null;
    ctx.save();
    ctx.translate(X(robot.x), Y(robot.y));
    ctx.rotate(-robot.th);
    ctx.globalAlpha = caido ? 0.55 : 1;
    if (estado.modoEnemigo !== 0) dibujarRayos(s, estado.sim);
    dibujarMiRobot(s, lin, color);
    ctx.restore();

    if (estado.fase === "colocar") {
      // Asa para girar el robot
      const [hx, hy] = aMundo(estado.robot, 0.09, 0);
      ctx.strokeStyle = color;
      ctx.lineWidth = 2;
      ctx.beginPath();
      ctx.moveTo(X(estado.robot.x), Y(estado.robot.y));
      ctx.lineTo(X(hx), Y(hy));
      ctx.stroke();
      ctx.beginPath();
      ctx.arc(X(hx), Y(hy), 7, 0, 2 * Math.PI);
      ctx.fillStyle = css("--surface");
      ctx.fill();
      ctx.stroke();
    }
    if (caido) {
      ctx.strokeStyle = css("--danger");
      ctx.lineWidth = 3;
      const x = X(robot.x),
        y = Y(robot.y),
        r = 0.07 * s;
      ctx.beginPath();
      ctx.moveTo(x - r, y - r);
      ctx.lineTo(x + r, y + r);
      ctx.moveTo(x + r, y - r);
      ctx.lineTo(x - r, y + r);
      ctx.stroke();
    }
    if (vista3d.activa) dibujar3D(robot, enemigo, lin, color);
  }

  return {
    dibujar,
    vista(tres) {
      vista3d.activa = tres && iniciar3D();
      dibujar();
    },
    destruir() {
      eventos.abort();
      if (vista3d.listo) {
        vista3d.escena.traverse((o) => {
          o.geometry?.dispose();
          const materiales = Array.isArray(o.material)
            ? o.material
            : [o.material];
          materiales.forEach((m) => m?.dispose());
        });
        vista3d.renderer.dispose();
      }
    },
  };
}
