#ifndef EGL_SRC_GLES2_SHADER_H_
#define EGL_SRC_GLES2_SHADER_H_

#include <GLES2/gl2.h>

#include "base/logging.h"

GLuint loadShader(GLenum shaderType, const char *source) {
  GLuint shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_FALSE) {
    LOG_E << "glCompileShader";
    exit(2);
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
    LOG_E << "glLinkProgram";
    exit(2);
  }
  glDeleteShader(vertexShader);
  glDeleteShader(fragShader);
  return program;
}

void deleteShaderProgram(GLuint shaderProgram) {
  glDeleteProgram(shaderProgram);
}

#endif // EGL_SRC_GLES2_SHADER_H_