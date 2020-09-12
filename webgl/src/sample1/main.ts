import { Renderer } from "./renderer";

function main(canvas: HTMLCanvasElement, gl: WebGLRenderingContext): void {
  const r = new Renderer(canvas, gl);
  r.initialize();
  r.draw();
}

export default main;