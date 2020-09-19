import { vec3 } from 'gl-matrix';
import { Shape } from './shape';

export function createCube(width: number): Shape {
  const vertices = [] as Shape['vertices'];
  const indices = [] as Shape['indices'];

  const a = [-width / 2, width / 2];

  for (const x of a) for (const y of a) for (const z of a) {
    vertices.push(vec3.fromValues(x, y, z));
  }

  for (let i = 0; i < 3; ++i)
    for (const s of a) {
      const plane = [] as Shape['vertices'];
      for (const x of a)
        for (const y of a) {
          const v = [x, y];
          v.splice(i, 0, s);
          plane.push(vec3.fromValues(...(v as [number, number, number])));
        }
      // console.log(plane);
      const ii = [0, 1, 2, 1, 2, 3].map((i) => vertices.findIndex((v) => vec3.equals(v, plane[i])));
      indices.push(ii.splice(0, 3) as [number, number, number]);
      indices.push(ii as [number, number, number]);
    }

  return { vertices, indices };
}