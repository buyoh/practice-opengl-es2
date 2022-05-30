#ifndef EGL_SRC_GLES2_TEXTURE_H_
#define EGL_SRC_GLES2_TEXTURE_H_

#include <optional>

#include <GLES2/gl2.h>

class GlES2Texture {
public:
  // GlES2Texture(GLuint texture = 0) : texture_(texture){};
  ~GlES2Texture() = default; // TODO: release?
  static std::optional<GlES2Texture> create();

  void initialize();

  void setBuffer(unsigned char *data, int frame_width, int frame_height,
                 GLint format = GL_RGBA);

  GLuint texture() const { return texture_; }
  void bindThisTexture() const;

private:
  GlES2Texture(GLuint texture) : texture_(texture){};
  GLuint texture_;
};

#endif // EGL_SRC_GLES2_TEXTURE_H_