attribute vec4 vPosition;
uniform mediump mat4 mRotation;
void main() {
  //
  gl_Position = mRotation * vPosition;
}
 