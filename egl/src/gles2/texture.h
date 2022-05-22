#ifndef EGL_SRC_GLES2_TEXTURE_H_
#define EGL_SRC_GLES2_TEXTURE_H_

#include <optional>

#include <GLES2/gl2.h>

class GlES2Texture {
public:
  // GlES2Texture(GLuint texture = 0) : texture_(texture){};
  static std::optional<GlES2Texture> create();

  void initialize();

  void setBuffer(unsigned char *data);
  void render();

private:
  GlES2Texture(GLuint texture) : texture_(texture){};
  GLuint texture_;
};

#endif // EGL_SRC_GLES2_TEXTURE_H_