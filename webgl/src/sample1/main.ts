import { Renderer } from "./renderer";

function main(canvas: HTMLCanvasElement, gl: WebGLRenderingContext): void {
  const r = new Renderer(canvas, gl);
  r.initialize();

  let last = 0;
  const render = (now: number) => {
    const delta = last ? now - last : 0;
    r.draw(delta / 1000);
    requestAnimationFrame(render);
    last = now;
  }
  requestAnimationFrame(render);
}

export default main;