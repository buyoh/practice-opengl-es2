#include <iostream>

#include "gles2/egl/eglext.h"

// <EGL/eglext.h>
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
// <GL/gl.h>
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

namespace {

#define LOOKUP_EXT_FUNCTION(F)                                                 \
  F = reinterpret_cast<decltype(F)>(eglGetProcAddress(#F));                    \
  if (!F) {                                                                    \
    std::clog << "eglGetProcAddress(" #F ")\n";                                \
    abort();                                                                   \
  }

void initializeEglExtensionFunctions() {
  std::clog << "initializeEglExtensionFunctions\n";
  LOOKUP_EXT_FUNCTION(eglCreateImageKHR)
  LOOKUP_EXT_FUNCTION(eglDestroyImageKHR)
  LOOKUP_EXT_FUNCTION(glEGLImageTargetTexture2DOES)
}

class Initializer {
public:
  // TODO: refactoring
  Initializer() { initializeEglExtensionFunctions(); }
} g_initializer;

} // namespace
