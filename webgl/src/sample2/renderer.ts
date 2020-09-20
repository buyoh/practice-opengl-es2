import { mat4 } from 'gl-matrix';
import { ColorList } from './color';
import { Entity } from './entity';
import { ShapeList } from './shape';


const vertexShaderSource = `
  attribute vec4 aVertexPosition;
  attribute vec4 aVertexColor;  // todo
  uniform vec4 uVertexColor;

  uniform mat4 uModelViewMatrix;
  uniform mat4 uProjectionMatrix;
  uniform float uStepUniformColor;

  varying lowp vec4 vColor;

  void main() {
    gl_Position = uProjectionMatrix * uModelViewMatrix * aVertexPosition;
    // vColor = aVertexColor;
    vColor = (1.0 - uStepUniformColor) * aVertexColor + uStepUniformColor * uVertexColor;
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
  // private colorBuffer: WebGLBuffer | null;

  private shapeList: ShapeList | null;
  private colorList: ColorList | null;

  private programInfo: any

  constructor(canvas: HTMLCanvasElement, gl: WebGLRenderingContext) {
    this.gl = gl;

    this.entities = [];
    this.verticesBuffer = null;
    this.indicesBuffer = null;
    this.shapeList = null;
    this.colorList = null;

    // set default camera
    this.projectionMatrix = mat4.create();
    mat4.perspective(this.projectionMatrix,
      45 * Math.PI / 180,
      canvas.clientWidth / canvas.clientHeight,
      0.1, 100);

    // this.colorBuffer = null;
  }

  registerEntity(entity: Entity): number {
    const i = this.entities.length;
    this.entities.push(entity);
    return i + 1;
  }

  initialize(shapeList: ShapeList, colorList: ColorList): boolean {
    if (!this.setupShader()) return false;
    if (!this.setupShapes(shapeList)) return false;
    if (!this.setupColors(colorList)) return false;
    this.shapeList = shapeList;
    this.colorList = colorList;
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
        vertexColor: gl.getUniformLocation(shaderProgram, 'uVertexColor'),
        stepUniformColor: gl.getUniformLocation(shaderProgram, 'uStepUniformColor'),
      },
    };
    return true;
  }

  private setupShapes(shapeList: ShapeList): boolean {
    const buffers = createPositionBufferOfShape(
      this.gl,
      shapeList.concatenatedVertices(),
      shapeList.concatenatedIndices());

    if (buffers === null) {
      console.error('webglbuffer construction failed');
      return false;
    }

    this.verticesBuffer = buffers.vertices;
    this.indicesBuffer = buffers.indices;
    return true;
  }

  private setupColors(colorList: ColorList): boolean {
    // const gl = this.gl;
    // {
    //   const colors = [
    //     0.0, 1.0, 1.0, 1.0
    //   ];

    //   this.colorBuffer = gl.createBuffer();
    //   gl.bindBuffer(gl.ARRAY_BUFFER, this.colorBuffer);
    //   gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
    // }
    return true;
  }

  draw(): void {
    if (this.programInfo === null || this.shapeList === null || this.colorList === null) {
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
        this.programInfo.attribLocations.vertexPosition,
        numComponents,
        type,
        normalize,
        stride,
        offset);
      gl.enableVertexAttribArray(
        this.programInfo.attribLocations.vertexPosition);
    }

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.indicesBuffer);

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
      this.projectionMatrix);

    {
      for (const entity of this.entities) {
        {  // set color
          const colorRange = this.colorList.getRange(entity.colorIndex);
          if (colorRange.type === 'single') {
            const color =
              this.colorList.concatenatedSingleColor().slice(
                colorRange.range.offset, colorRange.range.offset + colorRange.range.length);
            gl.uniform4fv(
              this.programInfo.uniformLocations.vertexColor,
              new Float32Array(color));
            gl.uniform1f(this.programInfo.uniformLocations.stepUniformColor, 1.0);
          }
        }

        const shapeRange = this.shapeList.getRange(entity.shapeIndex);

        const modelViewMatrix = mat4.create();
        mat4.fromRotationTranslation(modelViewMatrix, entity.rotation, entity.position);

        gl.uniformMatrix4fv(
          this.programInfo.uniformLocations.modelViewMatrix,
          false,
          modelViewMatrix);

        gl.drawElements(gl.TRIANGLES, shapeRange.length, gl.UNSIGNED_SHORT, shapeRange.offset);
      }
    }
  }
}


function createPositionBufferOfShape(
  gl: WebGLRenderingContext,
  vertices: ReadonlyArray<number>,
  indices: ReadonlyArray<number>)
  : { vertices: WebGLBuffer, indices: WebGLBuffer } | null {
  const verticesBuffer = gl.createBuffer();
  const indicesBuffer = gl.createBuffer();
  if (verticesBuffer === null || indicesBuffer === null)
    return null;

  gl.bindBuffer(gl.ARRAY_BUFFER, verticesBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);

  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indicesBuffer);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(indices), gl.STATIC_DRAW);

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