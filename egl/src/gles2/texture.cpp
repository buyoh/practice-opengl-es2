#include "gles2/texture.h"
#include "gles2/utils.h"

std::optional<GlES2Texture> GlES2Texture::create() {
  GLuint tex_handle = 0;
  // TODO: some textures
  glGenTextures(1, &tex_handle);
  return GlES2Texture(tex_handle);
}

void GlES2Texture::initialize() {

  glBindTexture(GL_TEXTURE_2D, texture_);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  // // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // // glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void GlES2Texture::setBuffer(unsigned char *data) {
  const int frame_width = 256;
  const int frame_height = 256;
  glBindTexture(GL_TEXTURE_2D, texture_);
  assert(checkGLES2Error());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  assert(checkGLES2Error());
}

void GlES2Texture::render() {
  // const int frame_width = 256;
  // const int frame_height = 256;

  // maybe NOT GLES2
  // glEnable(GL_TEXTURE_2D);
  // glBindTexture(GL_TEXTURE_2D, texture_);
  // glBegin(GL_QUADS);
  // glTexCoord2d(0, 0);
  // glVertex2i(200, 200);
  // glTexCoord2d(1, 0);
  // glVertex2i(200 + frame_width, 200);
  // glTexCoord2d(1, 1);
  // glVertex2i(200 + frame_width, 200 + frame_height);
  // glTexCoord2d(0, 1);
  // glVertex2i(200, 200 + frame_height);
  // glEnd();
  // glDisable(GL_TEXTURE_2D);
}