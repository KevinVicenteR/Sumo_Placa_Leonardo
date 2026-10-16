# Validación independiente X3, Rumania

Comparación alternativa: las tablas solicitadas no existen en HEAD 471815c05f17f1956ae445f203c6427e66214fb4. Se usaron las tablas del índice Git, cuyos blobs coinciden con el commit 39512ef (rama pesos_neuronales_rumania): TablaRedLogica 0e85bbe081c7b532efd27aa7d307ff3594a24a3c y TablaRedControl a250bf5d437023e2c2f9be40587d8b5964fba15b. Por tanto no es posible certificar literalmente una referencia extraída de HEAD.

Cada versión: 588 casillas (ROUNDS = ROUNDS_RUMANIA, 7 colocaciones × 7 rivales × 12 factores), 52 combates por casilla, 30 s, 30576 combates. Parámetros ROBOT de bateria.py (400 rpm). Sin --red ni parámetros adicionales.

Semillas: inicio de casilla i = 1000001 + 52*i, i=0..587; el simulador incrementa una unidad por combate. Rango 1000001–1030576, sin repetición entre casillas, disjunto de 1–52 de bateria.py con n=52. Se verificaron los 30576 pares por colocación, rival, factor y semilla entre versiones, y la secuencia exacta devuelta por cada lote.

| Grupo | N por versión | Referencia: gana / cae / propias | Entrenada: gana / cae / propias |
|---|---:|---:|---:|
| Total | 30576 | 39,39 % / 30,64 % / 108 | 47,75 % / 29,14 % / 106 |
| Ideal | 1456 | 89,01 % / 0,55 % / 1 | 99,66 % / 0,34 % / 1 |
| Modo 1 | 4368 | 41,78 % / 27,88 % / 1 | 50,94 % / 26,81 % / 2 |
| Modo 2 | 8736 | 43,25 % / 31,06 % / 60 | 50,72 % / 29,83 % / 64 |
| Modo 3 | 8736 | 37,37 % / 29,43 % / 20 | 47,13 % / 27,40 % / 16 |
| Modo 4 | 8736 | 36,37 % / 32,81 % / 27 | 43,82 % / 31,36 % / 24 |

Gana y cae son porcentajes sobre N; propias es número absoluto (cayo=1 y empujado!=1). Total incluye rival solo, igual que bateria.py. Ideal solo incluye factor Ideal y rivales quieto/deambula/embiste/flanquea: referencia 1296 victorias y 8 caídas, entrenada 1451 victorias y 5 caídas.

Mejora total: +8,3595 puntos porcentuales en victorias y -1,4946 en caídas, frente a +8,8/-1,5 publicados: confirmada dentro de ±1 punto bajo esta referencia alternativa. Los porcentajes totales de ambas versiones y los porcentajes por modo de la entrenada están a menos de 1 punto de las cifras proporcionadas. No se reproduce el conteo de caídas propias: referencia 108 frente a 73; entrenada 106 frente a 73. Por modo entrenada: 2/64/16/24 frente a 1/48/11/13. Ideal referencia obtiene 18 victorias más (1,24 puntos), fuera del margen de ±1; entrenada obtiene 3 victorias menos y 3 caídas más.

Compilación: script propio ejecutado en /private/tmp/codex_rumania_uEGA27, reutilizando simular.compilar({}) con RAIZ y BUILD temporales, FUENTES apuntando al src/ actual y simulacion/sim.cpp actual. Compilador c++, -std=c++17 -O2 -w, sin banderas adicionales, include temporal y simulacion/support actual. Cada versión copia include/ actual; referencia sustituye exclusivamente las dos tablas con git show :ruta. TacticaModo1 = TacticaArco en ambas. Comandos exactos y hashes SHA-256 de binarios e includes en codex_rumania_validacion_referencia.json y codex_rumania_validacion_entrenada.json.

Se comprobaron hashes de fuentes antes/después y estado Git: ninguna modificación fuera de resultados/codex_*. Sin commit, push ni hardware. CSV completos, resultados por casilla, scripts reproducibles y resumen de verificación junto a este informe.
