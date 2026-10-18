# Sumo X3 — Leonardo / JSumo XMotion, motores de 400 rpm

Firmware de competición (rounds de SECIHTI, espera de 5 s tras el START), simulador con la física compartida y una web React para explorar combates.

Los DIP eligen la ronda (DIP3, el lado: off = derecha, ON = izquierda):

| DIP1 | DIP2 | Modo | Archivo |
|---|---|---|---|
| off | off | 1: espalda con espalda (media vuelta y embestida) | `movimiento/rondas/Ronda1.h` |
| ON | off | 2: lado a lado (giro en el sitio hacia el rival y embestida) | `movimiento/rondas/Ronda2.h` |
| off | ON | 3: enfrentados, contragolpe | `movimiento/rondas/Ronda3.h` |
| ON | ON | 4: salida libre, arco | `movimiento/rondas/Libre.h` |

```text
firmware/
  include/                 Contratos, tablas de pesos y parámetros (Parametros.h es el índice)
    sensores/ParametrosSensores.h        sensores de enemigo y de piso
    hardware/ParametrosArranque.h        módulo de START, espera de 5 s y DIP
    movimiento/parametros/               motores, ataque, giro lateral, evasión,
                                         búsqueda, frenado, estimador de borde y PID
    movimiento/rondas/                   cada ronda: sus velocidades, sus tiempos y su lógica
    estrategia/ParametrosEstrategia.h    política aprendida y redes neuronales
  src/
    hardware/              XMotion, motores, módulo START y caja negra
    sensores/              Lectura, calibración y percepción
    movimiento/            Maniobras, control y límites del borde
      rondas/              Ronda1, Ronda2, Ronda3 y Libre
    estrategia/            Reglas, política y redes neuronales
    modos/                 Combate, diagnóstico y prueba de motores
    robot/                 Ciclo percibir → decidir → actuar
    main.cpp               Composición estática y entrada Arduino
simulacion/
  nucleo/                  fisica.h, sim.cpp, matematica.h y Arduino simulado
  python/                  Paquete: batería, simulación, redes y entrenadores
  web/                     Adaptador C++ para WebAssembly y soporte mínimo
  resultados/              Entradas activas y referencia de regresión
  bateria.py               Entrada compatible de la batería original
web/
  src/                     React + TypeScript, simulador y adaptador gráfico
  test/                    Pruebas de los módulos WebAssembly
  public/                  Módulos .wasm generados, sin seguimiento
scripts/                   Instalación, ejecución y comprobaciones
test/                     Pruebas nativas, Python y soporte de simulación
.github/workflows/ci.yml    Validación en Linux y artefacto web
```

## Arquitectura

`main.cpp` conecta las piezas del robot y el modo elegido por PlatformIO. `ControladorRobot` recibe percepción, estrategia, movimiento y motores por constructor. Las decisiones y maniobras usan sus contratos existentes; hardware y modos contienen los detalles de Arduino. No se añaden interfaces virtuales, asignaciones dinámicas ni RTTI al firmware. Se conserva su composición estática para respetar el límite de Flash del Leonardo.

El núcleo C++ simula sensores, motores y física sin incluir React ni Python. El paquete Python compila y ejecuta ese mismo firmware, evalúa combates y permite entrenar variantes de forma explícita. El adaptador `sim_web.cpp` expone ese núcleo a la web. React controla configuración y telemetría; `Simulador` administra el combate y `escena.js` dibuja las vistas 2D/3D conservadas. Three.js se empaqueta localmente: no se necesitan CDN ni traducción wasm2js. Cada combate obtiene una instancia WebAssembly nueva, con memoria independiente.

## Instalar y ejecutar

Requisitos: macOS con Homebrew o Linux con apt, Python ≥3.10, Node ≥18 (recomendado 22 LTS), compilador C++, LLVM/clang y `wasm-ld`. El instalador aprovecha lo ya instalado, instala PlatformIO 6.2.0 en `.venv` si hace falta y resuelve los paquetes de PlatformIO y npm. En Linux puede necesitar `sudo` para apt. Si falta red, informa los componentes pendientes y devuelve error; con las dependencias ya disponibles se puede trabajar sin conexión.

```bash
scripts/instalar.sh
scripts/correr.sh todo
scripts/correr.sh web
```

`todo` ejecuta pruebas, compila los cuatro entornos, corre el simulador y una batería corta, construye la web y prueba ambos WebAssembly. Termina con error si falla cualquier área, después de intentar las demás. `web` construye, prueba y deja Vite en `http://127.0.0.1:5173`; cambia el puerto con `PUERTO_WEB=5174`. Ningún subcomando carga firmware al robot.

También están disponibles `tests`, `compilar` y `simular`. `PYTHON` permite elegir intérprete y `PLATFORMIO_CORE_DIR` una caché escribible. Para compilar WebAssembly se puede indicar `LLVM_BIN`; `FIRMWARE_ANTERIOR` selecciona el commit de comparación (por defecto `10e7a1c`, firmware del torneo). Se necesita el historial Git de ese commit: una copia superficial debe obtenerlo primero.

La web conserva arrastre de ambos robots y orientación, cámara 3D, firmware nuevo/anterior, nueve rivales (incluidos tornado, contragolpe, woodpecker y esquivador), rounds, colocaciones de Rumania 1–4, velocidad, batería, llantas, polvo, sensores, cortes de START, telemetría y prueba de 200 combates con repetición de las caídas.

## Validar invariantes

```bash
# Referencia tomada ANTES de modificar el proyecto:
python3 simulacion/bateria.py --rumania --muestras 3000 --json /tmp/ref_antes.json
# Repetir tras los cambios:
python3 simulacion/bateria.py --rumania --muestras 3000 --json /tmp/ref_despues.json
python3 scripts/verificar_invariantes.py --antes /tmp/ref_antes.json --despues /tmp/ref_despues.json
# En una copia nueva, usar la referencia guardada:
python3 scripts/verificar_invariantes.py --antes simulacion/resultados/refactor_referencia.json --despues /tmp/ref_despues.json
python3 -m simulacion.python.verificar_arranques --referencia c8136d6
```

`scripts/verificar_invariantes.py` comprueba que una refactorización no cambia el robot: `--referencia COMMIT` compara las fuentes del firmware y `--antes/--despues` dos baterías casilla por casilla.

`pio test -e native`: 85 casos (rondas, maniobras, sensores, estimador, redes y caja negra). Las pruebas Python están en `test/python` y las de WebAssembly en `web/test`.

| Entorno | Flash usada / disponible |
| --- | ---: |
| leonardo | 27.344 / 28.672 B |
| monitoreo | 28.598 / 28.672 B |
| diagnostico | 11.618 / 28.672 B |
| prueba_motores | 6.202 / 28.672 B |

## Herramientas de simulación y entrenamiento

Se ejecutan como módulos desde la raíz, sin modificar `sys.path`:

```bash
python3 -m simulacion.python.simular --n 100
python3 -m simulacion.python.bateria --rumania --muestras 3000
python3 -m simulacion.python.diagnostico_caidas --muestras 1000
python3 -m simulacion.python.barrido_modo4 --modo 4 --n 2 Modo4VelocidadAvance=120
python3 -m simulacion.python.entrenar_jerarquica --help
```

También se conservan `barrido`, `evaluar`, `caidas_propias`, `optimizar`, `entrenar_redes`, `entrenar_politica`, `redes` y `verificar_arranques`; consulta `--help` en las herramientas de línea de comandos. Los entrenadores pueden escribir tablas si se ejecutan con sus opciones de publicación: no forman parte de `todo`. Los puntos de continuación y resultados nuevos se guardan en `simulacion/resultados/`.

El inventario completo de archivos retirados y el criterio de búsqueda están en [docs/limpieza.md](docs/limpieza.md). Se conservan las carpetas `candidatos_*` sin seguimiento. Los históricos eliminados siguen recuperables desde Git.

## Integración continua

GitHub Actions ejecuta pruebas nativas/Python, comprueba las fuentes protegidas, compila los cuatro entornos, ejecuta el simulador y la batería corta y compara la batería completa de Rumania. El segundo trabajo compila ambos WebAssembly con LLVM, construye React/TypeScript, ejecuta las pruebas web y guarda `web/dist` como artefacto. La caché de PlatformIO es `~/.platformio`; npm usa el archivo de bloqueo portable. Se ejecuta en push, pull request o manualmente. No despliega ni carga firmware.
