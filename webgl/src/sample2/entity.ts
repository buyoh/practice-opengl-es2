import { quat, vec3 } from 'gl-matrix';

export class Entity {
  shapeIndex: number;
  colorIndex: number;
  private position_: vec3;
  private rotation_: quat;

  constructor(shapeIndex: number, colorIndex: number, position: vec3 = vec3.create(), rotation: quat = quat.create()) {
    this.shapeIndex = shapeIndex;
    this.colorIndex = colorIndex;
    this.position_ = position;
    this.rotation_ = rotation;
  }

  get position(): vec3 {
    return this.position_;
  }

  get rotation(): quat {
    return this.rotation_;
  }
}
