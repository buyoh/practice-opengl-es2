import { IndexRange } from './shape';

export type Color = [number, number, number, number];

export class ColorList {
  private singleColor_: Array<number>;
  private verticesColor_: Array<number>
  private ranges_: Array<{ type: 'single' | 'vertice', range: IndexRange }>;

  constructor() {
    this.singleColor_ = [];
    this.verticesColor_ = [];
    this.ranges_ = [];
  }

  pushSingleColor(color: Color): number {
    const k = this.ranges_.length;
    const j = this.singleColor_.length;
    this.singleColor_.push(...color);
    this.ranges_.push({ type: 'single', range: { offset: j, length: 4 } });
    return k;
  }
  pushVerticeColor(colors: Array<Color>): number {
    const k = this.ranges_.length;
    const j = this.verticesColor_.length;
    this.verticesColor_.push(...colors.flat(2));
    this.ranges_.push({ type: 'vertice', range: { offset: j, length: colors.length * 4 } });
    return k;
  }

  concatenatedSingleColor(): ReadonlyArray<number> {
    return this.singleColor_;
  }
  concatenatedVerticesColor(): ReadonlyArray<number> {
    return this.verticesColor_;
  }
  getRange(index: number): { type: 'single' | 'vertice', range: IndexRange } {
    return { ...this.ranges_[index] };
  }
}
