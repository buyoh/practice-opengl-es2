#ifndef EGL_SRC_APP_APP_H_
#define EGL_SRC_APP_APP_H_

#include <EGL/egl.h>

class AppMain {
 public:
  AppMain() {}
  void startMainLoop(EGLDisplay display, EGLSurface surface);
};

#endif  // EGL_SRC_APP_APP_H_