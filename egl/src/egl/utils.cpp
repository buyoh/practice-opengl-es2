#include "egl/utils.h"
#include "base/logging.h"

namespace {
#define CASE_STR(value)                                                        \
  case value:                                                                  \
    return #value;
// https://stackoverflow.com/questions/38127022/is-there-a-standard-way-to-query-egl-error-string
const char *eglGetErrorString(EGLint error) {
  switch (error) {
    CASE_STR(EGL_SUCCESS)
    CASE_STR(EGL_NOT_INITIALIZED)
    CASE_STR(EGL_BAD_ACCESS)
    CASE_STR(EGL_BAD_ALLOC)
    CASE_STR(EGL_BAD_ATTRIBUTE)
    CASE_STR(EGL_BAD_CONTEXT)
    CASE_STR(EGL_BAD_CONFIG)
    CASE_STR(EGL_BAD_CURRENT_SURFACE)
    CASE_STR(EGL_BAD_DISPLAY)
    CASE_STR(EGL_BAD_SURFACE)
    CASE_STR(EGL_BAD_MATCH)
    CASE_STR(EGL_BAD_PARAMETER)
    CASE_STR(EGL_BAD_NATIVE_PIXMAP)
    CASE_STR(EGL_BAD_NATIVE_WINDOW)
    CASE_STR(EGL_CONTEXT_LOST)
  default:
    return "Unknown";
  }
}
} // namespace

bool checkEGLError() {
  auto e = eglGetError();
  if (e == EGL_SUCCESS)
    return true;
  LOG_E << "error = " << eglGetErrorString(e);
  return false;
}
