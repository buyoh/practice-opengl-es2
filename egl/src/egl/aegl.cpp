
#include "aegl.h"
#include "base/logging.h"

AEgl::AEgl() {}

AEgl::~AEgl() {
  eglDestroyContext(display_, context_);
  eglDestroySurface(display_, surface_);
  eglTerminate(display_);
}

bool AEgl::initialize(void *nativeDisplay, void *nativeWindow) {

  display_ =
      eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(nativeDisplay));

  if (display_ == EGL_NO_DISPLAY) {
    LOG_E << "eglGetDisplay";
    return false;
  }

  if (!eglInitialize(display_, nullptr, nullptr)) {
    LOG_E << "eglInitialize";
    return false;
  }

  EGLint attr[] = {EGL_BUFFER_SIZE, 16, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                   EGL_NONE};
  EGLConfig config = nullptr;
  EGLint numConfigs = 0;
  if (!eglChooseConfig(display_, attr, &config, 1, &numConfigs)) {
    LOG_E << "eglChooseConfig";
    return false;
  }
  if (numConfigs != 1) {
    LOG_E << "eglChooseConfig";
    std::cerr << "Error numConfigs." << std::endl;
    return false;
  }

  surface_ = eglCreateWindowSurface(
      display_, config, reinterpret_cast<EGLNativeWindowType>(nativeWindow),
      nullptr);
  if (surface_ == EGL_NO_SURFACE) {
    LOG_E << "eglCreateWindowSurface error=" << eglGetError();
    return false;
  }

  EGLint ctxattr[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, ctxattr);
  if (context_ == EGL_NO_CONTEXT) {
    LOG_E << "eglCreateContext error=" << eglGetError();
    return false;
  }
  eglMakeCurrent(display_, surface_, surface_, context_);

  return true;
}

EGLDisplay AEgl::getDisplay() { return display_; }

EGLSurface AEgl::getSurface() { return surface_; }
