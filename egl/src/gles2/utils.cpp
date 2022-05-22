
#include <GLES2/gl2.h>

#include "base/logging.h"
#include "gles2/utils.h"

bool checkGLES2Error() {
  GLenum err = glGetError();
  if (err == GL_NO_ERROR)
    return true;
  switch (err) {
  case GL_INVALID_ENUM:
    LOG_E << "error = GL_INVALID_ENUM";
    break;
  case GL_INVALID_VALUE:
    LOG_E << "error = GL_INVALID_VALUE";
    break;
  case GL_INVALID_OPERATION:
    LOG_E << "error = GL_INVALID_OPERATION";
    break;
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    LOG_E << "error = GL_INVALID_FRAMEBUFFER_OPERATION";
    break;
  case GL_OUT_OF_MEMORY:
    LOG_E << "error = GL_OUT_OF_MEMORY";
    break;

  default:
    LOG_E << "error = unknown: " << err;
    break;
  }
  return false;
}