import { vec3, ReadonlyVec3 } from 'gl-matrix';

export type Color = [number, number, number, number];

export type Shape = {
  vertices: Array<vec3>,
  vertexColors: Array<Color>,
  indices: Array<[number, number, number]>
};
export type ReadonlyShape = {
  readonly vertices: ReadonlyArray<ReadonlyVec3>,
  readonly vertexColors: ReadonlyArray<Color>,
  readonly indices: ReadonlyArray<[number, number, number]>
};

export type IndexRange = { offset: number, length: number };

export class ShapeList {
  private vertices_: Array<number>;
  private vertexColors_: Array<number>;
  private indices_: Array<number>;
  private ranges_: Array<IndexRange>;

  constructor() {
    this.vertices_ = [];
    this.vertexColors_ = [];
    this.indices_ = [];
    this.ranges_ = [];
  }

  push(shape: ReadonlyShape): number {
    const indicesOffset = this.indices_.length;
    const verticesOffset = this.vertices_.length / 3;
    const m = shape.indices.length;  // assert m >= n
    const k = this.ranges_.length;
    if (shape.vertices.length !== shape.vertexColors.length) {
      throw Error('assertion vertices.length === vertexColors.length');
    }
    this.vertices_.push(...shape.vertices.map(v => Array.from(v)).flat(1));
    this.vertexColors_.push(...shape.vertexColors.flat(1));
    this.indices_.push(...shape.indices.flat(1).map(e => e + verticesOffset));
    this.ranges_.push({ offset: indicesOffset, length: m * 3 });
    return k;
  }

  concatenatedVertices(): ReadonlyArray<number> {
    return this.vertices_;
  }
  concatenatedVertexColors(): ReadonlyArray<number> {
    return this.vertexColors_;
  }
  concatenatedIndices(): ReadonlyArray<number> {
    return this.indices_;
  }
  getRange(index: number): IndexRange {
    return { ...this.ranges_[index] };
  }
}
