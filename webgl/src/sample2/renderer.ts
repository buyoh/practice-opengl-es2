import { mat4 } from 'gl-matrix';
import { Entity } from './entity';
import { ShapeList } from './shape';


const vertexShaderSource = `
  attribute vec4 aVertexPosition;
  attribute vec4 aVertexColor;

  uniform mat4 uModelViewMatrix;
  uniform mat4 uProjectionMatrix;
  uniform float uStepColor;

  varying lowp vec4 vColor;

  void main() {
    gl_Position = uProjectionMatrix * uModelViewMatrix * aVertexPosition;
    vColor = uStepColor * aVertexColor;
  }
`;

const fragmentShaderSource = `
  varying lowp vec4 vColor;

  void main() {
    // gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
    gl_FragColor = vColor;
  }
`;

export class Renderer {
  private gl: WebGLRenderingContext

  /* public */ projectionMatrix: mat4;

  private entities: Array<Entity>
  private verticesBuffer: WebGLBuffer | null;
  private indicesBuffer: WebGLBuffer | null;
  private colorBuffer: WebGLBuffer | null;

  private shapeList: ShapeList | null;

  private shaderProgram: WebGLProgram | null;
  private locations: {
    attribute: {
      vertexPosition: number,
      vertexColor: number
    },
    uniform: {
      projectionMatrix: WebGLUniformLocation,
      modelViewMatrix: WebGLUniformLocation,
      stepColor: WebGLUniformLocation
    },
  } | null;

  constructor(canvas: HTMLCanvasElement, gl: WebGLRenderingContext) {
    this.gl = gl;

    this.entities = [];
    this.verticesBuffer = null;
    this.indicesBuffer = null;
    this.colorBuffer = null;
    this.shapeList = null;
    this.shaderProgram = null;
    this.locations = null;

    // set default camera
    this.projectionMatrix = mat4.create();
    mat4.perspective(this.projectionMatrix,
      45 * Math.PI / 180,
      canvas.clientWidth / canvas.clientHeight,
      0.1, 100);

  }

  registerEntity(entity: Entity): number {
    const i = this.entities.length;
    this.entities.push(entity);
    return i + 1;
  }

  initialize(shapeList: ShapeList): boolean {
    if (!this.setupShader()) return false;
    if (!this.setupShapes(shapeList)) return false;
    this.shapeList = shapeList;
    return true;
  }

  private setupShader(): boolean {
    const gl = this.gl;

    // initialze shaders
    const vertexShader = loadShader(gl, gl.VERTEX_SHADER, vertexShaderSource);
    const fragmentShader = loadShader(gl, gl.FRAGMENT_SHADER, fragmentShaderSource);
    if (vertexShader === null || fragmentShader === null) {
      console.error('shader initialization failed');
      return false;
    }

    // create the shader program
    const program = createShaderProgram(gl, [vertexShader, fragmentShader]);
    if (program === null) {
      console.error('shaderProgram initialization failed');
      return false;
    }
    this.shaderProgram = program;

    const unwrap = <T>(x: T | null): T => {
      if (x === null) throw Error('getLocation failed');
      return x;
    };

    this.locations = {
      attribute: {
        vertexPosition: gl.getAttribLocation(program, 'aVertexPosition'),
        vertexColor: gl.getAttribLocation(program, 'aVertexColor'),
      },
      uniform: {
        projectionMatrix: unwrap(gl.getUniformLocation(program, 'uProjectionMatrix')),
        modelViewMatrix: unwrap(gl.getUniformLocation(program, 'uModelViewMatrix')),
        stepColor: unwrap(gl.getUniformLocation(program, 'uStepColor')),
      },
    };

    return true;
  }

  private setupShapes(shapeList: ShapeList): boolean {
    this.verticesBuffer = createBindedBuffer(this.gl, this.gl.ARRAY_BUFFER,
      new Float32Array(shapeList.concatenatedVertices()));
    if (this.verticesBuffer === null)
      return false;

    this.indicesBuffer = createBindedBuffer(this.gl, this.gl.ELEMENT_ARRAY_BUFFER,
      new Uint16Array(shapeList.concatenatedIndices()));
    if (this.indicesBuffer === null)
      return false;

    this.colorBuffer = createBindedBuffer(this.gl, this.gl.ARRAY_BUFFER,
      new Float32Array(shapeList.concatenatedVertexColors()));
    if (this.colorBuffer === null)
      return false;

    return true;
  }

  draw(): void {
    if (this.shapeList === null || this.locations === null) {
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

    // Tell WebGL how to pull out the positions from the position
    // buffer into the vertexPosition attribute.
    {
      const numComponents = 3;  // pull out 2 values per iteration
      const type = gl.FLOAT;    // the data in the buffer is 32bit floats
      const normalize = false;  // don't normalize
      const stride = 0;         // how many bytes to get from one set of values to the next
      // 0 = use type and numComponents above
      const offset = 0;         // how many bytes inside the buffer to start from
      gl.bindBuffer(gl.ARRAY_BUFFER, this.verticesBuffer);
      gl.vertexAttribPointer(
        this.locations.attribute.vertexPosition,
        numComponents,
        type,
        normalize,
        stride,
        offset);
      gl.enableVertexAttribArray(
        this.locations.attribute.vertexPosition);
    }

    {
      gl.bindBuffer(gl.ARRAY_BUFFER, this.colorBuffer);
      gl.vertexAttribPointer(
        this.locations.attribute.vertexColor,
        4,  // numComponents
        gl.FLOAT,
        false,  // normalize
        0,  // stride
        0);
      gl.enableVertexAttribArray(this.locations.attribute.vertexColor);
    }

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indicesBuffer);

    gl.useProgram(this.shaderProgram);

    gl.uniformMatrix4fv(
      this.locations.uniform.projectionMatrix,
      false,
      this.projectionMatrix);
    gl.uniform1f(this.locations.uniform.stepColor, 1);

    for (const entity of this.entities) {
      const shapeRange = this.shapeList.getRange(entity.shapeIndex);

      const modelViewMatrix = mat4.create();
      mat4.fromRotationTranslation(modelViewMatrix, entity.rotation, entity.position);

      gl.uniformMatrix4fv(
        this.locations.uniform.modelViewMatrix,
        false,
        modelViewMatrix);

      gl.drawElements(gl.TRIANGLES, shapeRange.length, gl.UNSIGNED_SHORT, 2 * shapeRange.offset);
    }
  }
}


function createBindedBuffer(gl: WebGLRenderingContext, target: number,
  data: ArrayBufferView | ArrayBuffer, usage = gl.STATIC_DRAW): WebGLBuffer | null {
  const buffer = gl.createBuffer();
  if (buffer === null) {
    console.warn('createBuffer failed');
    return null;
  }
  gl.bindBuffer(target, buffer);
  gl.bufferData(target, data, usage);
  return buffer;
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