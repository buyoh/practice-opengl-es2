import { vec3 } from "gl-matrix";
import { Renderer } from "./renderer";
import { createCube } from "./shape_factory";

let running = false;

function start(canvas: HTMLCanvasElement, gl: WebGLRenderingContext): void {
  const r = new Renderer(canvas, gl);
  const shape = createCube(1);
  const position = vec3.fromValues(0, 0, -5);
  r.registerEntity(shape, position);
  r.initialize();

  let last = 0;
  const render = (now: number) => {
    const delta = last ? now - last : 0;
    r.draw(delta / 1000);
    if (running)
      requestAnimationFrame(render);
    last = now;
  }
  running = true;
  requestAnimationFrame(render);
}

function stop() {
  running = false;
}

export default { start, stop };