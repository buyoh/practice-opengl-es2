// cloned from: https://qiita.com/y-tsutsu/items/1e88212b8532fc693c3c

#include <GLES2/gl2.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <iostream>
#include <math.h>
#include <unistd.h>
#define degree2radian(degree) ((degree * M_PI) / 180.0F)

#include <EGL/egl.h>
#include <X11/Xlib.h>
#include <iostream>
int initializeEGL(Display *xdisp, Window &xwindow, EGLDisplay &display,
                  EGLContext &context, EGLSurface &surface) {
  display = eglGetDisplay(static_cast<EGLNativeDisplayType>(xdisp));
  if (display == EGL_NO_DISPLAY) {
    std::cerr << "Error eglGetDisplay." << std::endl;
    return -1;
  }
  if (!eglInitialize(display, nullptr, nullptr)) {
    std::cerr << "Error eglInitialize." << std::endl;
    return -1;
  }

  EGLint attr[] = {EGL_BUFFER_SIZE, 16, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                   EGL_NONE};
  EGLConfig config = nullptr;
  EGLint numConfigs = 0;
  if (!eglChooseConfig(display, attr, &config, 1, &numConfigs)) {
    std::cerr << "Error eglChooseConfig." << std::endl;
    return -1;
  }
  if (numConfigs != 1) {
    std::cerr << "Error numConfigs." << std::endl;
    return -1;
  }

  surface = eglCreateWindowSurface(display, config, xwindow, nullptr);
  if (surface == EGL_NO_SURFACE) {
    std::cerr << "Error eglCreateWindowSurface. " << eglGetError() << std::endl;
    return -1;
  }

  EGLint ctxattr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxattr);
  if (context == EGL_NO_CONTEXT) {
    std::cerr << "Error eglCreateContext. " << eglGetError() << std::endl;
    return -1;
  }
  eglMakeCurrent(display, surface, surface, context);

  return 0;
}

void destroyEGL(EGLDisplay &display, EGLContext &context, EGLSurface &surface) {
  eglDestroyContext(display, context);
  eglDestroySurface(display, surface);
  eglTerminate(display);
}

GLuint loadShader(GLenum shaderType, const char *source) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    std::cerr << "Error glCompileShader." << std::endl;
    exit(EXIT_FAILURE);
  }
  return shader;
}

GLuint createProgram(const char *vshader, const char *fshader) {
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshader);
  GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fshader);
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    std::cerr << "Error glLinkProgram." << std::endl;
    exit(EXIT_FAILURE);
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragShader);
  return program;
}

void deleteShaderProgram(GLuint shaderProgram) {
  glDeleteProgram(shaderProgram);
}

void mainloop(Display *xdisplay, EGLDisplay display, EGLSurface surface) {
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
    XPending(xdisplay);

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

int main(int argc, char *argv[]) {
  Display *xdisplay = XOpenDisplay(nullptr);
  if (xdisplay == nullptr) {
    std::cerr << "Error XOpenDisplay." << std::endl;
    exit(EXIT_FAILURE);
  }

  Window xwindow = XCreateSimpleWindow(
      xdisplay, DefaultRootWindow(xdisplay), 100, 100, 640, 480, 1,
      BlackPixel(xdisplay, 0), WhitePixel(xdisplay, 0));

  XMapWindow(xdisplay, xwindow);

  EGLDisplay display = nullptr;
  EGLContext context = nullptr;
  EGLSurface surface = nullptr;
  if (initializeEGL(xdisplay, xwindow, display, context, surface) < 0) {
    std::cerr << "Error initializeEGL." << std::endl;
    exit(EXIT_FAILURE);
  }

  mainloop(xdisplay, display, surface);

  destroyEGL(display, context, surface);
  XDestroyWindow(xdisplay, xwindow);
  XCloseDisplay(xdisplay);

  return 0;
}