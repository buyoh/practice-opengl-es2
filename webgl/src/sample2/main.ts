import { mat4, quat, vec3 } from 'gl-matrix';
import { Entity } from './entity';
import { Renderer } from './renderer';
import { Color, ShapeList } from './shape';
import { createCubeShape } from './shape_factory';

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
  const cubeColor1 = [
    [0, 0, 1, 1], [0, 1, 0, 1],
    [0, 1, 1, 1], [1, 0, 0, 1],
    [1, 0, 1, 1], [1, 1, 0, 1],
  ] as [Color, Color, Color, Color, Color, Color];
  const cubeColor2 = [
    [0.2, 0.2, 0.2, 1], [0.3, 0.3, 0.3, 1],
    [0.4, 0.4, 0.4, 1], [0.6, 0.6, 0.6, 1],
    [0.7, 0.7, 0.7, 1], [0.8, 0.8, 0.8, 1],
  ] as [Color, Color, Color, Color, Color, Color];
  const sCube1 = shapeList.push(createCubeShape(1, cubeColor1));
  const sCube2 = shapeList.push(createCubeShape(0.2, cubeColor2));
  const sCube3 = shapeList.push(createCubeShape(0.4, cubeColor1));

  const renderer = new Renderer(canvas, gl);
  const e1 = new Entity(sCube1, vec3.fromValues(-1.5, 0, 0));
  const e2 = new Entity(sCube2, vec3.fromValues(1.5, 0, 0));
  const e3 = new Entity(sCube3, vec3.fromValues(0, 0, 0));
  const e4 = new Entity(sCube2, vec3.fromValues(0, 0, 1.5));

  renderer.initialize(shapeList);

  renderer.registerEntity(e1);
  renderer.registerEntity(e2);
  renderer.registerEntity(e3);
  renderer.registerEntity(e4);

  // let last = 0;
  const render = (now: number) => {

    const cameraRot = 0.001 * now;
    const projection = createBaseProjectionMatrix(canvas);

    const camera = mat4.lookAt(mat4.create(),
      vec3.fromValues(5.0 * Math.cos(cameraRot), 2.0 * Math.sin(0.2 * cameraRot), 5.0 * Math.sin(cameraRot)),
      vec3.fromValues(0, 0, 0),
      vec3.fromValues(0, 1, 0));

    mat4.mul(projection, projection, camera);
    renderer.projectionMatrix = projection;

    quat.fromEuler(e1.rotation, 0, 0.06 * now, 0.1 * now);
    quat.fromEuler(e2.rotation, 0, 0.06 * now, 0.1 * now);

    renderer.draw();

    if (running)
      requestAnimationFrame(render);
    // last = now;
  };

  running = true;
  requestAnimationFrame(render);
}

function stop(): void {
  running = false;
}

export default { start, stop };