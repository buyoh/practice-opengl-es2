import { mat4, vec3 } from 'gl-matrix';
import { Entity, Shape } from './entity';


const vertexShaderSource = `
  attribute vec4 aVertexPosition;
  // attribute vec4 aVertexColor;

  uniform mat4 uModelViewMatrix;
  uniform mat4 uProjectionMatrix;

  // varying lowp vec4 vColor;  // !!

  void main() {
    gl_Position = uProjectionMatrix * uModelViewMatrix * aVertexPosition;
    // vColor = aVertexColor;
  }
`;

const fragmentShaderSource = `
  // varying lowp vec4 vColor;  // !!

  void main() {
    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    // gl_FragColor = vColor;
  }
`;

export class Renderer {

  private canvas: HTMLCanvasElement
  private gl: WebGLRenderingContext
  private time: number

  private entities: Array<Entity>
  private entityShapeBuffer: Array<{ vertices: WebGLBuffer, indices: WebGLBuffer }>

  // private colorBuffer: WebGLBuffer | null
  private programInfo: any

  constructor(canvas: HTMLCanvasElement, gl: WebGLRenderingContext) {
    this.canvas = canvas;
    this.gl = gl;
    this.time = 0;

    this.entities = [];
    this.entityShapeBuffer = [];

    // this.colorBuffer = null;
  }

  registerEntity(shape: Shape, position: vec3): number {
    const i = this.entities.length;
    const buffers = createPositionBufferOfShape(this.gl, shape);
    if (buffers === null) {
      console.error('webglbuffer construction failed');
      return -1;  // bad workaround
    }
    this.entities.push(new Entity(shape, position));
    this.entityShapeBuffer.push(buffers);
    return i + 1;
  }

  getEntityPosition(entityId: number): vec3 {
    return this.entities[entityId - 1].position;
  }

  initialize(): boolean {
    const gl = this.gl;

    // initialze shaders
    const vertexShader = loadShader(gl, gl.VERTEX_SHADER, vertexShaderSource);
    const fragmentShader = loadShader(gl, gl.FRAGMENT_SHADER, fragmentShaderSource);
    if (vertexShader === null || fragmentShader === null) {
      console.error('shader initialization failed');
      return false;
    }

    // create the shader program
    const shaderProgram = createShaderProgram(gl, [vertexShader, fragmentShader]);
    if (shaderProgram === null) {
      console.error('shaderProgram initialization failed');
      return false;
    }

    // Shader の attribute のロケーションを取得
    this.programInfo = {
      program: shaderProgram,
      attribLocations: {
        vertexPosition: gl.getAttribLocation(shaderProgram, 'aVertexPosition'),
        vertexColor: gl.getAttribLocation(shaderProgram, 'aVertexColor'),
      },
      uniformLocations: {
        projectionMatrix: gl.getUniformLocation(shaderProgram, 'uProjectionMatrix'),
        modelViewMatrix: gl.getUniformLocation(shaderProgram, 'uModelViewMatrix'),
      },
    };

    return true;
  }

  draw(deltaTime: number): void {
    if (this.programInfo === null) {
      console.warn('it may not initialize app');
      return;
    }
    const gl = this.gl;

    gl.clearColor(0.0, 0.0, 0.0, 1.0);  // Clear to black, fully opaque
    gl.clearDepth(1.0);                 // Clear everything
    gl.enable(gl.DEPTH_TEST);           // Enable depth testing
    gl.depthFunc(gl.LEQUAL);            // Near things obscure far things

    // Clear the canvas before we start drawing on it.

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // Create a perspective matrix, a special matrix that is
    // used to simulate the distortion of perspective in a camera.
    // Our field of view is 45 degrees, with a width/height
    // ratio that matches the display size of the canvas
    // and we only want to see objects between 0.1 units
    // and 100 units away from the camera.

    const fieldOfView = 45 * Math.PI / 180;   // in radians
    const aspect = this.canvas.clientWidth / this.canvas.clientHeight;
    const zNear = 0.1;
    const zFar = 100.0;
    const projectionMatrix = mat4.create();

    // note: glmatrix.js always has the first argument
    // as the destination to receive the result.
    mat4.perspective(projectionMatrix,
      fieldOfView,
      aspect,
      zNear,
      zFar);

    // Set the drawing position to the "identity" point, which is
    // the center of the scene.
    const modelViewMatrix = mat4.create();

    // Now move the drawing position a bit to where we want to
    // start drawing the square.

    mat4.translate(modelViewMatrix,     // destination matrix
      modelViewMatrix,     // matrix to translate
      [-0.0, 0.0, -6.0]);  // amount to translate

    mat4.rotateZ(modelViewMatrix,  // destination matrix
      modelViewMatrix,  // matrix to rotate
      this.time * 1.2);   // amount to rotate in radians
    mat4.rotateX(modelViewMatrix,  // destination matrix
      modelViewMatrix,  // matrix to rotate
      this.time);   // amount to rotate in radians

    // Tell WebGL how to pull out the positions from the position
    // buffer into the vertexPosition attribute.
    {
      const numComponents = 3;  // pull out 2 values per iteration
      const type = gl.FLOAT;    // the data in the buffer is 32bit floats
      const normalize = false;  // don't normalize
      const stride = 0;         // how many bytes to get from one set of values to the next
      // 0 = use type and numComponents above
      const offset = 0;         // how many bytes inside the buffer to start from
      gl.bindBuffer(gl.ARRAY_BUFFER, this.entityShapeBuffer[0].vertices);
      gl.vertexAttribPointer(
        this.programInfo.attribLocations.vertexPosition,
        numComponents,
        type,
        normalize,
        stride,
        offset);
      gl.enableVertexAttribArray(
        this.programInfo.attribLocations.vertexPosition);
    }

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.entityShapeBuffer[0].indices);

    // {
    //   const numComponents = 4;
    //   const type = gl.FLOAT;
    //   const normalize = false;
    //   const stride = 0;
    //   const offset = 0;
    //   gl.bindBuffer(gl.ARRAY_BUFFER, this.colorBuffer);
    //   gl.vertexAttribPointer(
    //     this.programInfo.attribLocations.vertexColor,
    //     numComponents,
    //     type,
    //     normalize,
    //     stride,
    //     offset);
    //   gl.enableVertexAttribArray(
    //     this.programInfo.attribLocations.vertexColor);
    // }

    // Tell WebGL to use our program when drawing

    gl.useProgram(this.programInfo.program);

    // Set the shader uniforms

    gl.uniformMatrix4fv(
      this.programInfo.uniformLocations.projectionMatrix,
      false,
      projectionMatrix);
    gl.uniformMatrix4fv(
      this.programInfo.uniformLocations.modelViewMatrix,
      false,
      modelViewMatrix);

    {
      const offset = 0;
      const vertexCount = this.entities[0].shape.indices.length * 3;
      // gl.drawArrays(gl.TRIANGLE_STRIP, offset, vertexCount);
      gl.drawElements(gl.TRIANGLES, vertexCount, gl.UNSIGNED_SHORT, offset);
    }

    this.time += deltaTime;
  }
}


function createPositionBufferOfShape(gl: WebGLRenderingContext, shape: Shape): { vertices: WebGLBuffer, indices: WebGLBuffer } | null {
  const verticesBuffer = gl.createBuffer();
  const indicesBuffer = gl.createBuffer();
  if (verticesBuffer === null || indicesBuffer === null)
    return null;

  const vv = shape.vertices.map((v) => Array.from(v)).flat(2);
  gl.bindBuffer(gl.ARRAY_BUFFER, verticesBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vv), gl.STATIC_DRAW);

  const ii = shape.indices.map((v) => Array.from(v)).flat(2);
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indicesBuffer);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(ii), gl.STATIC_DRAW);

  return {
    vertices: verticesBuffer,
    indices: indicesBuffer
  };
}

function loadShader(gl: WebGLRenderingContext, type: number, source: string) {
  const shader = gl.createShader(type);
  if (shader === null) return null;

  gl.shaderSource(shader, source);
  gl.compileShader(shader);

  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    console.error('An error occurred compiling the shaders: ', gl.getShaderInfoLog(shader));
    gl.deleteShader(shader);
    return null;
  }

  return shader;
}

function createShaderProgram(gl: WebGLRenderingContext, shaders: Array<WebGLShader>) {
  const shaderProgram = gl.createProgram();
  if (shaderProgram === null) {
    console.error('createProgram failed');
    return null;
  }
  for (const shader of shaders)
    gl.attachShader(shaderProgram, shader);
  gl.linkProgram(shaderProgram);

  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
    console.error('Unable to initialize the shader program: ', gl.getProgramInfoLog(shaderProgram));
    return null;
  }

  return shaderProgram;
}