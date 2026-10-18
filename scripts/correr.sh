#!/usr/bin/env bash
# No contiene comandos de carga al robot ni operaciones remotas de Git.
set -euo pipefail
RAIZ="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$RAIZ"
export PATH="$RAIZ/.venv/bin:$PATH"
PYTHON="${PYTHON:-python3}"
tests() {
 pio test -e native
 "$PYTHON" -m unittest discover -s test/python -v
}
compilar() { pio run -e leonardo -e monitoreo -e diagnostico -e prueba_motores; }
simular() {
 "$PYTHON" -m simulacion.python.simular --n 3 --dur 2
 "$PYTHON" simulacion/bateria.py --rumania --n 1 --dur 2 --json simulacion/build/bateria_corta.json
}
web() {
 npm --prefix web run build
 npm --prefix web test
}
case "${1:-todo}" in
 tests) tests ;;
 compilar) compilar ;;
 simular) simular ;;
 web) web; exec npm --prefix web run dev -- --port "${PUERTO_WEB:-5173}" --strictPort ;;
 todo)
  # Intenta cada área aunque otra falle, para informar todo lo comprobable.
  estado=0
  for tarea in tests compilar simular web; do
   printf '\n=== %s ===\n' "$tarea"
   # Subshell independiente: mantiene errexit dentro de cada función.
   set +e
   (set -e; "$tarea")
   resultado=$?
   set -e
   if [[ "$resultado" -ne 0 ]]; then printf 'FALLÓ: %s (%s)\n' "$tarea" "$resultado" >&2; estado=1; fi
  done
  exit "$estado"
  ;;
 *) echo 'Uso: scripts/correr.sh tests | compilar | simular | web | todo' >&2; exit 2 ;;
esac
