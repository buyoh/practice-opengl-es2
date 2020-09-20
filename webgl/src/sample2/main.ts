import { mat4, quat, vec3 } from 'gl-matrix';
import { ColorList } from './color';
import { Entity } from './entity';
import { Renderer } from './renderer';
import { ShapeList } from './shape';
import { createCube } from './shape_factory';

function createBaseProjectionMatrix(canvas: HTMLCanvasElement): mat4 {
  // Create a perspective matrix, a special matrix that is
  // used to simulate the distortion of perspective in a camera.
  const fieldOfView = 45 * Math.PI / 180;
  const aspect = canvas.clientWidth / canvas.clientHeight;
  const zNear = 0.1;
  const zFar = 100.0;

  return mat4.perspective(mat4.create(),
    fieldOfView, aspect, zNear, zFar);
}

let running = false;

function start(canvas: HTMLCanvasElement, gl: WebGLRenderingContext): void {
  const shapeList = new ShapeList();
  const sCube1 = shapeList.push(createCube(1));
  const sCube2 = shapeList.push(createCube(0.2));
  const sCube3 = shapeList.push(createCube(0.4));

  const colorList = new ColorList();
  const cCyan = colorList.pushSingleColor([0, 1, 1, 1]);
  const cYellow = colorList.pushSingleColor([1, 1, 0, 1]);

  const renderer = new Renderer(canvas, gl);
  const e1 = new Entity(sCube1, cCyan, vec3.fromValues(-1.5, 0, -5));
  const e2 = new Entity(sCube2, cCyan, vec3.fromValues(1.5, 0, -5));
  const e3 = new Entity(sCube3, cYellow, vec3.fromValues(0, 0, -5));

  renderer.initialize(shapeList, colorList);

  renderer.registerEntity(e1);
  renderer.registerEntity(e2);
  renderer.registerEntity(e3);

  let last = 0;
  const render = (now: number) => {
    const delta = last ? now - last : 0;

    const cameraRot = 0.001 * now;
    const projection = createBaseProjectionMatrix(canvas);

    const camera = mat4.targetTo(mat4.create(),
      vec3.fromValues(0.0 + 5.0 * Math.cos(cameraRot), 0, -5.0 + 5.0 * Math.sin(cameraRot)),
      vec3.fromValues(0, 0, -5.0),
      vec3.fromValues(0, 1, 0));

    mat4.mul(projection, projection, camera);
    renderer.projectionMatrix = projection;

    quat.fromEuler(e1.rotation, 0, 0.06 * now, 0.1 * now);
    quat.fromEuler(e2.rotation, 0, 0.06 * now, 0.1 * now);

    renderer.draw();

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