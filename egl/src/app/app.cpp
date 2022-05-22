// cloned from: https://qiita.com/y-tsutsu/items/1e88212b8532fc693c3c

#include <GLES2/gl2.h>

#include <algorithm>
#include <iostream>
#include <math.h>
#include <unistd.h>
#include <vector>
#define degree2radian(degree) ((degree * M_PI) / 180.0F)

#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <iostream>

#include "egl/aegl.h"
#include "gles2/shader.h"
#include "gles2/texture.h"
#include "gles2/utils.h"
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

  const char *texture_vshader = R"(
        attribute vec4 a_position;
        attribute vec2 a_uv;
        varying mediump vec2 v_uv;
        void main() {
            gl_Position = a_position;
            v_uv = a_uv;
        }
    )";

  const char *texture_fshader = R"(
        uniform sampler2D u_texture;
        varying mediump vec2 v_uv;
        void main() {
            gl_FragColor = texture2D(u_texture, v_uv);
        }
    )";

  GlES2ShaderProgram shader_program;
  if (!shader_program.initialize(vshader, fshader)) {
    return;
  }
  GLuint program = shader_program.program();

  GlES2ShaderProgram texture_shader_program;
  if (!texture_shader_program.initialize(texture_vshader, texture_fshader)) {
    return;
  }
  GLuint texture_program = texture_shader_program.program();

  const GLfloat vertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};
  GLint gvPositionHandle = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(gvPositionHandle);
  GLint gmRotationHandle = glGetUniformLocation(program, "mRotation");

  GLint a_position_handle = glGetAttribLocation(texture_program, "a_position");
  glEnableVertexAttribArray(a_position_handle);
  GLint a_uv_handle = glGetAttribLocation(texture_program, "a_uv");
  glEnableVertexAttribArray(a_uv_handle);
  GLint u_texture_handle = glGetUniformLocation(texture_program, "u_texture");

  std::vector<unsigned char> image_buffer(256 * 256 * 4);
  std::fill(image_buffer.begin(), image_buffer.end(), 0x80);
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 64; ++x) {
      int p = y * 256 * 4 + x * 4;
      image_buffer[p] = 0xff;
      image_buffer[p + 1] = 0;
      image_buffer[p + 2] = 0;
    }
  }

  GlES2Texture texture_holder = *GlES2Texture::create(); // unwrap
  texture_holder.setBuffer(image_buffer.data());

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

    const GLfloat aa_position[] = {
        -0.75f, 0.75f,  //
        -0.75f, -0.75f, //
        0.75f,  0.75f,  //
        0.75f,  -0.75f, //
    };
    const GLfloat aa_uv[] = {
        0.0f, 0.0f, //
        0.0f, 1.0f, //
        1.0f, 0.0f, //
        1.0f, 1.0f, //
    };

    glUseProgram(texture_program);
    glUniform1i(u_texture_handle, 0);
    glVertexAttribPointer(a_position_handle, 2, GL_FLOAT, GL_FALSE, 0,
                          aa_position);
    glVertexAttribPointer(a_uv_handle, 2, GL_FLOAT, GL_FALSE, 0, aa_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(program);
    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    eglSwapBuffers(display, surface);
    degree = (degree + 1) % 360;
    usleep(16600);
  }
}

} // namespace App