import { vec3 } from 'gl-matrix';

export type Shape = { vertices: Array<vec3>, indices: Array<[number, number, number]> };

export class Entity {
  private shape_: Shape;
  private position_: vec3;
  constructor(shape: Shape, position: vec3) {
    this.shape_ = shape;
    this.position_ = position;
  }

  get shape(): Shape {
    return this.shape_;
  }

  get position(): vec3 {
    return this.position_;
  }
}
