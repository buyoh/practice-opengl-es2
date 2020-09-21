import { vec3 } from 'gl-matrix';
import { Color, Shape } from './shape';

export function createCubeShape(width: number, colors: [Color, Color, Color, Color, Color, Color]): Shape {
  const vertices = [] as Shape['vertices'];
  const vertexColors = colors.map(e => [e, e, e, e]).flat(1);
  const indices = [] as Shape['indices'];

  // note: 面ごとに色を変えたいならば、面ごとに頂点を作成する必要がある

  const a = [-width / 2, width / 2];

  for (let i = 0; i < 3; ++i)
    for (const s of a) {
      const k = vertices.length;
      for (const x of a)
        for (const y of a) {
          const v = [x, y];
          v.splice(i, 0, s);
          vertices.push(vec3.fromValues(...(v as [number, number, number])));
        }
      indices.push([k, k + 1, k + 2], [k + 2, k + 1, k + 3]);
    }

  return { vertices, vertexColors, indices };
}
