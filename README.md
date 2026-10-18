# Minisumo Leonardo / JSumo XMotion

Firmware de competición, simulador con la física compartida y una web React para explorar combates. La reorganización parte de `pesos_neuronales_rumania` (`c8136d6`) y conserva las fuentes del firmware byte a byte: mismas decisiones, tablas de pesos, parámetros, telemetría y arranque mediante START sin espera de 5 segundos en Rumania.

```text
firmware/
  include/                 Contratos, parámetros y tablas
  src/
    hardware/              XMotion, motores, módulo START y caja negra
    sensores/              Lectura, calibración y percepción
    movimiento/            Maniobras, control y límites del borde
      rumania/             Modos 1-4 de Rumania: cada uno con sus velocidades y su lógica
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

La batería usa 7 colocaciones × 11 rivales × 12 factores = 924 casillas. Redondea 3.000 muestras a 4 combates por casilla: 3.696 combates. La referencia y el resultado del refactor coinciden íntegramente: **2.101 victorias, 1.010 caídas, 9 caídas propias**. El verificador compara el JSON completo, además de todas las cabeceras y fuentes C++ del firmware contra `c8136d6`; falla ante cualquier diferencia. Los arranques protegidos también mantienen sus 4.636 ciclos comparados. Los números guardados se midieron en macOS ARM64; CI genera antes y después en el mismo runner para evitar diferencias numéricas entre compiladores o plataformas.

`pio test -e native`: 94 casos, 83 correctos y 11 omitidos por tácticas desactivadas en la configuración actual. Las pruebas Python están en `test/python`; una prueba antigua dependía de pesos entrenados que ya no garantizaban su hipótesis, y ahora utiliza una red controlada sin modificar las tablas. Las pruebas WebAssembly verifican memoria independiente, repetibilidad, todos los modos/rivales, factores y movimiento antes de 5 segundos.

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
