// cloned from: https://qiita.com/y-tsutsu/items/1e88212b8532fc693c3c

#include <math.h>
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include "base/logging.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <X11/Xlib.h>

#include "app/shader/shader.h"
#include "egl/aegl.h"
#include "gles2/egl/dma_buffer_texture.h"
#include "gles2/shader.h"
#include "gles2/texture.h"
#include "gles2/utils.h"
#include "v4l2/v4l2_device.h"
#include "window/awindow_x11.h"

#include <EGL/eglext.h>
#include <GL/gl.h>

#include "app/app.h"

#define USE_V4L2 0

namespace {

const char* vshader = _binary_src_app_shader_a_vert_start;
const char* fshader = _binary_src_app_shader_a_frag_start;

const char* texture_vshader = R"(
        attribute vec4 a_position;
        attribute vec2 a_uv;
        varying mediump vec2 v_uv;
        void main() {
            gl_Position = a_position;
            v_uv = a_uv;
        }
    )";

#if USE_V4L2
const char* texture_fshader = R"(
        #extension GL_OES_EGL_image_external : require
        uniform samplerExternalOES u_texture;
        varying mediump vec2 v_uv;
        void main() {
            gl_FragColor = texture2D(u_texture, v_uv);
        }
    )";
#else
const char* texture_fshader = R"(
      uniform sampler2D u_texture;
      varying mediump vec2 v_uv;
      void main() {
          gl_FragColor = texture2D(u_texture, v_uv);
      }
  )";
#endif

}  // namespace

// TODO: classize regarding dtor
void AppMain::startMainLoop(EGLDisplay display, EGLSurface surface) {
  //
  GlES2ShaderProgram shader_program;
  if (!shader_program.initialize(vshader, fshader)) {
    LOG_E << "shader_program";
    return;
  }
  GLuint program = shader_program.program();

  GlES2ShaderProgram texture_shader_program;
  if (!texture_shader_program.initialize(texture_vshader, texture_fshader)) {
    LOG_E << "texture_shader_program";
    return;
  }
  GLuint texture_program = texture_shader_program.program();

  //
  const GLfloat vertices[] = {0.0f,  0.5f,  0.0f,  //
                              -0.5f, -0.5f, 0.0f,  //
                              0.5f,  -0.5f, 0.0f};

  const GLfloat vertices2[] = {0.0f,  0.5f,  0.1f,  //
                               -0.5f, -0.5f, 0.1f,  //
                               0.5f,  -0.5f, 0.1f};

  //
  GLint gvPositionHandle = glGetAttribLocation(program, "vPosition");
  glEnableVertexAttribArray(gvPositionHandle);
  GLint gmRotationHandle = glGetUniformLocation(program, "mRotation");
  GLint gvColorHandle = glGetUniformLocation(program, "vColor");

  GLint a_position_handle = glGetAttribLocation(texture_program, "a_position");
  glEnableVertexAttribArray(a_position_handle);
  GLint a_uv_handle = glGetAttribLocation(texture_program, "a_uv");
  glEnableVertexAttribArray(a_uv_handle);
  GLint u_texture_handle = glGetUniformLocation(texture_program, "u_texture");

  //

#if USE_V4L2
  //
  V4L2Device v4l2;
  if (!v4l2.open("/dev/video0"))
    return;
  v4l2.getParameterVideoCapture();  // TODO:

  //
  DMABufferTexture dma;
  if (!dma.initialize(display, v4l2, 1280, 720)) {
    return;
  }
  dma.queue(v4l2, 0);
#else

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
  GlES2Texture texture_holder = *GlES2Texture::create();  // unwrap
  texture_holder.initialize();
  texture_holder.setBuffer(image_buffer.data(), 256, 256, GL_RGBA);
#endif

  //
  // GlES2Texture depth_texture_holder = *GlES2Texture::create(); // unwrap
  // depth_texture_holder.setBuffer(nullptr, 256, 256, GL_DEPTH_COMPONENT);

  // 隠面消去
  glEnable(GL_DEPTH_TEST);
  // glDepthFunc(GL_LESS);

  // カリング(裏の描画を回避)
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);

  // 透明でないものを全て描き、
  // glDepthMask を GL_FALSE にします。
  // 半透明なものを描き、
  // glDepthMask を GL_TRUE に戻します。
  // glDepthMask(GL_FALSE);

  for (int counter = 0;; ++counter) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClearColor(0.25f, 0.25f, 0.5f, 1.0f);

    // render texture
    const GLfloat aa_position[] = {
        -0.75f, 0.0f,    //
        -0.75f, -0.75f,  //
        0.0f,   0.0f,    //
        0.0f,   -0.75f,  //
    };
    const GLfloat aa_uv[] = {
        0.0f, 0.0f,  //
        0.0f, 1.0f,  //
        1.0f, 0.0f,  //
        1.0f, 1.0f,  //
    };

    glUseProgram(texture_program);
    glUniform1i(u_texture_handle, 0 /* texture unit id */);
    glVertexAttribPointer(a_position_handle, 2, GL_FLOAT, GL_FALSE, 0,
                          aa_position);
    glVertexAttribPointer(a_uv_handle, 2, GL_FLOAT, GL_FALSE, 0, aa_uv);

#if USE_V4L2
    // TODO: other thread
    dma.dequeue(v4l2, (counter + 1) % 2);
    dma.queue(v4l2, (counter + 1) % 2);

    dma.bindTexture(counter % 2);
#else
    texture_holder.bindThisTexture();
#endif
    // glViewport();
    glDisable(GL_DEPTH_TEST);
    // Draw as 2D
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_DEPTH_TEST);

    // render triangle
    double angle = M_PI * 2 * counter / 180;
    const GLfloat matrix[] = {static_cast<GLfloat>(cos(angle)),
                              0.0f,
                              static_cast<GLfloat>(sin(angle)),
                              0.0f,
                              0.0f,
                              1.0f,
                              0.0f,
                              0.0f,
                              static_cast<GLfloat>(-sin(angle)),
                              0.0f,
                              static_cast<GLfloat>(cos(angle)),
                              0.0f,
                              0.0f,
                              0.0f,
                              0.0f,
                              1.0f};
    const GLfloat color[] = {0.3f, 0.8f, 0.3f, 1.0f};

    // triangle 1
    glUseProgram(program);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);
    glUniform4fv(gvColorHandle, 1, color);
    // glViewport();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    const GLfloat color2[] = {0.8f, 0.3f, 0.3f, 1.0f};

    // triangle 2
    glUseProgram(program);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0,
                          vertices2);
    glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);
    glUniform4fv(gvColorHandle, 1, color2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    eglSwapBuffers(display, surface);
    usleep(16600);
  }
#if USE_V4L2
  v4l2.stopV4L2stream();
#endif
  VLOG(0) << "done";
}
