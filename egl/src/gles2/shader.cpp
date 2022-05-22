
#include <optional>

#include "gles2/shader.h"

namespace {

GLuint loadShader(GLenum shaderType, const char *source) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    LOG_E << "glCompileShader";
    return 0;
  }
  return shader;
}

GLuint createProgram(const char *vshader, const char *fshader) {
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshader);
  if (vertexShader == 0)
    return 0;
  GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fshader);
  if (fragShader == 0)
    return 0;
  GLuint program = glCreateProgram();
  if (program == 0) {
    LOG_E << "glCreateProgram";
    return 0;
  }
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    // TODO: get error details
    LOG_E << "glLinkProgram";
    return 0;
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragShader);
  return program;
}

void deleteShaderProgram(GLuint shaderProgram) {
  glDeleteProgram(shaderProgram);
}

} // namespace

GlES2ShaderProgram::~GlES2ShaderProgram() {
  if (program_) {
    deleteShaderProgram(program_);
  }
}

bool GlES2ShaderProgram::initialize(const char *vshader, const char *fshader) {
  program_ = createProgram(vshader, fshader);
  return !!program_;
}
