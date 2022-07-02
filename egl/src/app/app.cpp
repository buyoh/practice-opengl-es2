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
#define USE_OFFSCREEN 1
#define USE_BUFFER 1

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
#if USE_BUFFER
  // const GLfloat vertices_b[] = {0.5f,  0.5f,  0.0f,  //
  //                               -0.5f, 0.5f,  0.0f,  //
  //                               -0.5f, -0.5f, 0.0f,  //
  //                               0.5f,  -0.5f, 0.0f};
  // const GLushort indices_b[] = {0, 1, 2, 3};
#endif

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

#if USE_OFFSCREEN
  GLint defaultFrameBuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFrameBuffer);
  VLOG(0) << "default frame buffer: " << defaultFrameBuffer;
#endif

#if USE_OFFSCREEN
  // todo: snake_case
  int offscreenTextureWidth = 1024;
  int offscreenTextureHeight = 768;
  //
  GLuint offscreenColorTexture;  // TODO: dtor
  glGenTextures(1, &offscreenColorTexture);
  assert(checkGLES2Error());
  glBindTexture(GL_TEXTURE_2D, offscreenColorTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, offscreenTextureWidth,
               offscreenTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  assert(checkGLES2Error());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //
  GLuint offscreenDepthBuffer;  // TODO: dtor
  glGenRenderbuffers(1, &offscreenDepthBuffer);
  assert(checkGLES2Error());
  glBindRenderbuffer(GL_RENDERBUFFER, offscreenDepthBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
                        offscreenTextureWidth, offscreenTextureHeight);
  assert(checkGLES2Error());
  //
  GLuint offscreenFrameBuffer;  // TODO: dtor
  glGenFramebuffers(1, &offscreenFrameBuffer);
  assert(checkGLES2Error());
  glBindFramebuffer(GL_FRAMEBUFFER, offscreenFrameBuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         offscreenColorTexture, 0);
  assert(checkGLES2Error());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, offscreenDepthBuffer);
  assert(checkGLES2Error());

  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  //
  glGenTextures(1, 0);
  glGenRenderbuffers(1, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
#if USE_BUFFER
  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  assert(checkGLES2Error());
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 12, vertices_b,
  //              GL_STATIC_DRAW);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 9, vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //
  // GLuint indices_buffer;
  // glGenBuffers(1, &indices_buffer);
  // assert(checkGLES2Error());
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 4, indices_b,
  //              GL_STATIC_DRAW);
  // glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

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
#if USE_OFFSCREEN
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFrameBuffer);
    glViewport(0, 0, offscreenTextureWidth, offscreenTextureHeight);
    glClearColor(1.0f, 1.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    glClearColor(0.25f, 0.25f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
#if USE_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0,
                          reinterpret_cast<void*>(0 /* beginning index*/));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, vertices);
#endif
    glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);
    glUniform4fv(gvColorHandle, 1, color);
    // glViewport();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    const GLfloat color2[] = {0.8f, 0.3f, 0.3f, 1.0f};

#if USE_OFFSCREEN
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFrameBuffer);
#endif
    // triangle 2
    glUseProgram(program);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0,
                          vertices2);
    glUniformMatrix4fv(gmRotationHandle, 1, GL_FALSE, matrix);
    glUniform4fv(gvColorHandle, 1, color2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

#if USE_OFFSCREEN
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
#endif

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
#elif USE_OFFSCREEN
    glBindTexture(GL_TEXTURE_2D, offscreenColorTexture);
#else
    texture_holder.bindThisTexture();

#endif
    // glViewport();
    glDisable(GL_DEPTH_TEST);
    // Draw as 2D
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_DEPTH_TEST);

    eglSwapBuffers(display, surface);
    usleep(16600);
  }
#if USE_V4L2
  v4l2.stopV4L2stream();
#endif
  VLOG(0) << "done";
}
