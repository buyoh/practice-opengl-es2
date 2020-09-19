import { Renderer } from './renderer';

let running = false;

function start(canvas: HTMLCanvasElement, gl: WebGLRenderingContext): void {
  const r = new Renderer(canvas, gl);
  r.initialize();

  let last = 0;
  const render = (now: number) => {
    const delta = last ? now - last : 0;
    r.draw(delta / 1000);
    if (running)
      requestAnimationFrame(render);
    last = now;
  };
  running = true;
  requestAnimationFrame(render);
}

function stop(): void {
  running = false;
}

export default { start, stop };