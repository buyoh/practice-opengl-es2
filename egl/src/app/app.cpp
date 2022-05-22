// cloned from: https://qiita.com/y-tsutsu/items/1e88212b8532fc693c3c

#include <GLES2/gl2.h>

#include <algorithm>
#include <iostream>
#include <math.h>
#include <unistd.h>
#define degree2radian(degree) ((degree * M_PI) / 180.0F)

#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <iostream>

#include "egl/aegl.h"
#include "gles2/shader.h"
#include "window/awindow_x11.h"

namespace App {

void mainloop(EGLDisplay display, EGLSurface surface) {
  const char *vshader = R"(
        attribute vec4 vPosition;
        uniform mediump mat4 mRotation;
        void main() {
            gl_Position = mRotation * vPosition;
        }
    )";

  const char *fshader = R"(
        precision mediump float;
        void main() {
            gl_FragColor = vec4(0.3, 0.8, 0.3, 1.0);
        }
    )";

  GLuint program = createProgram(vshader, fshader);
  glUseProgram(program);
  const GLfloat vertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
  GLint gvPositionHandle = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(gvPositionHandle);
  GLint gmRotationHandle = glGetUniformLocation(program, "mRotation");
  int degree = 0;
  while (true) {
    const GLfloat matrix[] = {static_cast<GLfloat>(cos(degree2radian(degree))),
                              0.0f,
                              static_cast<GLfloat>(sin(degree2radian(degree))),
                              0.0f,
                              0.0f,
                              1.0f,
                              0.0f,
                              0.0f,
                              static_cast<GLfloat>(-sin(degree2radian(degree))),
                              0.0f,
                              static_cast<GLfloat>(cos(degree2radian(degree))),
                              0.0f,
                              0.0f,
                              0.0f,
                              0.0f,
                              1.0f};

    glClearColor(0.25f, 0.25f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    eglSwapBuffers(display, surface);
    degree = (degree + 1) % 360;
    usleep(16600);
  }
  deleteShaderProgram(program);
}

} // namespace App