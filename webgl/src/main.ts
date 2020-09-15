import Sample1 from './sample1/main';

interface Sample {
  start(canvas: HTMLCanvasElement, gl: WebGLRenderingContext): void
  stop(): void
}

const samples: { [i: string]: Sample | null } = {
  sample1: Sample1
};
const defaultSelectedSample = 'sample1';


function setupSampleSelector() {
  let currentlySelected = sessionStorage.getItem('selectedSample') || defaultSelectedSample;

  const dom = document.getElementById('selector');
  if (dom === null) return;
  for (const key of Object.keys(samples)) {
    const d = document.createElement('option');
    d.appendChild(document.createTextNode(key));
    if (key === currentlySelected)
      d.defaultSelected = true;
    dom.appendChild(d);
  }

  dom.addEventListener('change', (e: Event) => {
    samples[currentlySelected]?.stop();
    const key = (e.target as HTMLSelectElement).value as string;
    currentlySelected = key;
    sessionStorage.setItem('selectedSample', key);

    startSample(samples[currentlySelected]);
  });

  startSample(samples[currentlySelected]);
}


function startSample(sample: Sample | null): void {
  if (sample === null)
    return;
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

  sample.start(canvas, gl);
}

window.addEventListener('load', setupSampleSelector);
