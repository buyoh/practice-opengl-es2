import { vec3, ReadonlyVec3 } from 'gl-matrix';

export type Shape = {
  vertices: Array<vec3>,
  indices: Array<[number, number, number]>
};
export type ReadonlyShape = {
  readonly vertices: ReadonlyArray<ReadonlyVec3>,
  readonly indices: ReadonlyArray<[number, number, number]>
};

export type IndexRange = { offset: number, length: number };

export class ShapeList {
  private vertices_: Array<number>;
  private indices_: Array<number>;
  private ranges_: Array<IndexRange>;

  constructor() {
    this.vertices_ = [];
    this.indices_ = [];
    this.ranges_ = [];
  }

  push(shape: ReadonlyShape): number {
    const i = this.vertices_.length;
    // const n = shape.vertices.length;
    const m = shape.indices.length;  // assert m >= n
    const k = this.ranges_.length;
    this.vertices_.push(...Array.from(shape.vertices.map(v => Array.from(v)).flat(2)));
    this.indices_.push(...Array.from(shape.indices.flat(2)));
    this.ranges_.push({ offset: i, length: m * 3 });
    return k;
  }

  concatenatedVertices(): ReadonlyArray<number> {
    return this.vertices_;
  }
  concatenatedIndices(): ReadonlyArray<number> {
    return this.indices_;
  }
  concatenatedRanges(): ReadonlyArray<IndexRange> {
    return this.ranges_;
  }
  // getRange(index: number): IndexRange {
  //   return { ...this.ranges_[index] };
  // }
}