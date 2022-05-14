#include <EGL/egl.h>
#include <X11/Xlib.h>

#include "base/logging.h"

int initializeEGL(Display *xdisp, Window &xwindow, EGLDisplay &display,
                  EGLContext &context, EGLSurface &surface) {
  display = eglGetDisplay(static_cast<EGLNativeDisplayType>(xdisp));
  if (display == EGL_NO_DISPLAY) {
    LOG(0) << "Error eglGetDisplay.";
    return -1;
  }
  if (!eglInitialize(display, nullptr, nullptr)) {
    LOG(0) << "Error eglInitialize.";
    return -1;
  }

  EGLint attr[] = {EGL_BUFFER_SIZE, 16, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                   EGL_NONE};
  EGLConfig config = nullptr;
  EGLint numConfigs = 0;
  if (!eglChooseConfig(display, attr, &config, 1, &numConfigs)) {
    LOG(0) << "Error eglChooseConfig.";
    return -1;
  }
  if (numConfigs != 1) {
    LOG(0) << "Error numConfigs.";
    return -1;
  }

  surface = eglCreateWindowSurface(
      display, config, static_cast<EGLNativeWindowType>(xwindow), nullptr);
  if (surface == EGL_NO_SURFACE) {
    LOG(0) << "Error eglCreateWindowSurface. " << eglGetError();
    return -1;
  }

  EGLint ctxattr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxattr);
  if (context == EGL_NO_CONTEXT) {
    LOG(0) << "Error eglCreateContext. " << eglGetError();
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