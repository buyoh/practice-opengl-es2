#include "gles2/texture.h"
#include "base/logging.h"
#include "gles2/utils.h"

std::optional<GlES2Texture> GlES2Texture::create() {
  GLuint tex_handle = 0;
  // TODO: some textures
  glGenTextures(1, &tex_handle);
  VLOG(0) << "textureid=" << tex_handle;
  return GlES2Texture(tex_handle);
}

void GlES2Texture::initialize() {
  // なんか初期化し忘れてても動く
  glBindTexture(GL_TEXTURE_2D, texture_);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void GlES2Texture::setBuffer(unsigned char *data, int frame_width,
                             int frame_height, GLint format) {
  glBindTexture(GL_TEXTURE_2D, texture_);
  assert(checkGLES2Error());
  glTexImage2D(GL_TEXTURE_2D, 0, format, frame_width, frame_height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  assert(checkGLES2Error());
}

void GlES2Texture::bindThisTexture() const {
  glBindTexture(GL_TEXTURE_2D, texture_);
}