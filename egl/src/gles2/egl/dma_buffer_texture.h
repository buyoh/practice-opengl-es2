#ifndef EGL_SRC_GLES2_EGL_DMA_BUFFER_TEXTURE_H_
#define EGL_SRC_GLES2_EGL_DMA_BUFFER_TEXTURE_H_

#include <vector>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

class DMABufferTexture {
public:
  DMABufferTexture() {}
  ~DMABufferTexture() = default; // TODO: release?

  bool initialize(EGLDisplay egl_display, int width, int height);

  const std::vector<GLuint> &textures() const { return textures_; }

  void bindTexture(int idx) const;

  // for testing // WIP
  void dequeue() const;
  void queue() const;

private:
  std::vector<GLuint> textures_;
};

#endif // EGL_SRC_GLES2_EGL_DMA_BUFFER_TEXTURE_H_