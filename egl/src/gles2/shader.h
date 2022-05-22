#ifndef EGL_SRC_GLES2_SHADER_H_
#define EGL_SRC_GLES2_SHADER_H_

#include <GLES2/gl2.h>

#include "base/logging.h"

class GlES2ShaderProgram {

public:
  GlES2ShaderProgram() : program_(0){};
  ~GlES2ShaderProgram();
  bool initialize(const char *vshader, const char *fshader);

  GLuint program() const { return program_; }

private:
  GLuint program_;
};

#endif // EGL_SRC_GLES2_SHADER_H_