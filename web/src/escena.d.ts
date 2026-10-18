import type { Estado } from "./simulador";
export function crearEscena(
  canvas: HTMLCanvasElement,
  canvas3d: HTMLCanvasElement,
  estado: Estado,
): {
  dibujar(): void;
  vista(tres: boolean): void;
  destruir(): void;
};
