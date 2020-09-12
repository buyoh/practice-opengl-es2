import Sample1 from './sample1/main';

function main(): void {
  const canvas = document.querySelector('#canvas') as (HTMLCanvasElement | null);
  if (canvas === null) {
    console.error('canvas not found');
    return;
  }
  const gl = canvas.getContext('webgl');
  if (gl === null) {
    console.error('your browser may not support webgl');
    return;
  }

  // TODO: コンテンツが増えたら考える。
  Sample1(canvas, gl);
}

window.addEventListener('load', main);
