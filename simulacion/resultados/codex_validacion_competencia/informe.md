# Validación independiente de competencia X3

Fuente: worktree x3-sin-arrancador, rama X3_pesos_neuronales_sin_arrancador. Copia del árbol de trabajo con cambios sin commit. Referencia: solo las dos tablas sustituidas por git show pesos_neuronales_sin_arrancador:<ruta>.

Batería normal: 5 rounds × 7 rivales × 12 factores × 72 semillas = 30 240 combates por versión; duración 30 s; parámetros ROBOT de bateria.py (400 rpm). Semillas 424242–424313 en cada casilla, compartidas entre versiones. Ideal: quieto/deambula/embiste/flanquea, factor Ideal. Caída propia: cayo=1 y empujado!=1. CSV comprobados: 420 casillas, semillas exactas y agregados consistentes.

| Grupo | N por versión | Referencia gana / cae / propias | Entrenada gana / cae / propias |
|---|---:|---:|---:|
| Total | 30240 | 35.57% / 26.08% / 72 | 42.54% / 24.39% / 70 |
| Ideal | 1440 | 74.44% / 0.28% / 0 | 95.69% / 0.28% / 0 |
| Modo 1 | 6048 | 33.43% / 23.73% / 12 | 42.31% / 22.02% / 11 |
| Modo 2 | 12096 | 30.19% / 23.19% / 17 | 37.81% / 20.71% / 15 |
| Modo 3 | 6048 | 47.69% / 30.62% / 32 | 51.69% / 29.70% / 33 |
| Modo 4 | 6048 | 36.36% / 29.70% / 11 | 43.07% / 28.82% / 11 |

Se confirma la mejora global dentro de ±1 punto: +6,964 puntos en victorias (Claude +7,0) y −1,693 puntos en caídas (Claude −1,4). Cada tasa global también queda a menos de 0,4 puntos de Claude.

Ideal: referencia 1072/1440 victorias y 4 caídas (Claude 1083 y 8); entrenada 1378/1440 y 4 caídas (Claude 1386 y 8). Mejora +21,25 puntos frente a +21,04 de Claude.

Discrepancias por modo entrenado respecto a Claude (puntos gana/cae; diferencia de propias): m1 +1,11/−0,28/+2; m2 −0,29/+0,01/0; m3 −0,01/+0,40/+10; m4 +1,27/−1,48/−4. Por tanto, no todas las tasas por modo se reproducen dentro de ±1 punto. Caídas propias totales: referencia 72 frente a 56 (+16), entrenada 70 frente a 62 (+8). En estas semillas la entrenada tiene 2 propias menos que la referencia, mientras Claude observó 6 más.

Se cambiaron las semillas según lo solicitado; las discrepancias observadas son sensibles a esa muestra. No se ejecutaron las semillas originales para atribuir causalidad adicional. Validación del simulador, sin hardware, sin commit/push. Fuente leída sin modificaciones. Por restricciones de escritura del entorno, estos resultados se guardaron en el worktree simulaciones-30k-muestras-6a1c63. El script original se ejecutó en /tmp/codex_validacion_competencia/validar.py y se conserva una copia aquí.
