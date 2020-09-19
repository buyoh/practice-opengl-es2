import { quat, vec3 } from 'gl-matrix';
import { Entity } from './entity';
import { Renderer } from './renderer';
import { ShapeList } from './shape';
import { createCube } from './shape_factory';

let running = false;

function start(canvas: HTMLCanvasElement, gl: WebGLRenderingContext): void {
  const shapeList = new ShapeList();
  const sCube1 = shapeList.push(createCube(1));
  const sCube2 = shapeList.push(createCube(0.8));

  const r = new Renderer(canvas, gl);
  const e1 = new Entity(sCube1, vec3.fromValues(-1.5, 0, 0));
  const e2 = new Entity(sCube2, vec3.fromValues(1.5, 0, 0));

  r.initialize(shapeList);

  r.registerEntity(e1);
  r.registerEntity(e2);

  let last = 0;
  const render = (now: number) => {
    const delta = last ? now - last : 0;
    quat.fromEuler(e1.rotation, 0, 0.06 * now, 0.1 * now);
    quat.fromEuler(e2.rotation, 0, 0.06 * now, 0.1 * now);
    r.draw();
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