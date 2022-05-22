
#include <optional>
#include <string>

#include "gles2/shader.h"

namespace {

std::string getShaderInfoLog(GLuint shader) {
  std::string msg;
  GLint len = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
  if (len >= 1) {
    msg.resize(len, ' ');
    glGetShaderInfoLog(shader, len, nullptr, msg.data());
  }
  return msg;
}

std::string getProgramInfoLog(GLuint program) {
  std::string msg;
  GLint len = 0;
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
  if (len >= 1) {
    msg.resize(len, ' ');
    glGetProgramInfoLog(program, len, nullptr, msg.data());
  }
  return msg;
}

GLuint loadShader(GLenum shaderType, const char *source) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    auto info_log = getShaderInfoLog(shader);
    LOG_E << "glCompileShader: " << info_log;
    return 0;
  }
  return shader;
}

GLuint createProgram(const char *vshader, const char *fshader) {
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vshader);
  if (vertexShader == 0)
    return 0;
  GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fshader);
  if (fragShader == 0) {
    glDeleteShader(vertexShader);
    return 0;
  }
  GLuint program = glCreateProgram();
  if (program == 0) {
    LOG_E << "glCreateProgram";
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    return 0;
  }
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);
  GLint linkStatus = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    // TODO: get error details
    auto info_log = getProgramInfoLog(program);
    LOG_E << "glLinkProgram: " << info_log;
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
    glDeleteProgram(program);
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
