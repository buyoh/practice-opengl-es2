attribute vec4 vPosition;
// uniform vec4 vColor;
uniform mediump mat4 mRotation;
// varying mediump vec4 vvColor;
void main() {
  //
  gl_Position = mRotation * vPosition;
  // vvColor = vColor;
}
 