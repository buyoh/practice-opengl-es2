//
#include <cassert>
#include <optional>
#include <vector>
//
#include <libdrm/drm.h>
#include <libdrm/drm_fourcc.h>
//
#include <EGL/egl.h>
#include <GLES2/gl2.h>
//
// GL_TEXTURE_EXTERNAL_OES in <GLES2/gl2ext.h>
#include <GLES2/gl2ext.h>
//
#include "base/logging.h"
#include "egl/utils.h"
#include "gles2/egl/eglext.h"
#include "gles2/utils.h"
//
#include "gles2/egl/dma_buffer_texture.h"

// https://manual.atmark-techno.com/armadillo-810/armadillo-810_product_manual_ja-1.3.0/ch14.html

namespace {

std::vector<GLuint> bindTextures(EGLDisplay egl_display,
                                 const std::vector<int> &buffer_fds, int width,
                                 int height) {

  std::vector<GLuint> textures(buffer_fds.size());
  glGenTextures(buffer_fds.size(), textures.data());
  for (int i = 0; i < buffer_fds.size(); ++i) {

    EGLint attrs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                      //
                      EGL_WIDTH, width,
                      //
                      EGL_HEIGHT, height,
                      //
                      EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_YUYV,
                      //
                      EGL_DMA_BUF_PLANE0_FD_EXT, buffer_fds[i],
                      //
                      EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
                      //
                      EGL_DMA_BUF_PLANE0_PITCH_EXT, width,
                      //
                      EGL_NONE};

    EGLImageKHR image = eglCreateImageKHR(egl_display, EGL_NO_CONTEXT,
                                          EGL_LINUX_DMA_BUF_EXT, NULL, attrs);

    // EGL_BAD_PARAMETER, EGL_BAD_MATCH -> not supported
    assert(checkEGLError());
    assert(image != EGL_NO_IMAGE_KHR);

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textures[i]);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    assert(checkGLES2Error());
    // glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    // assert(checkGLES2Error());
    // TODO: check error
  }
  return textures;
}

} // namespace

bool DMABufferTexture::initialize(EGLDisplay egl_display,
                                  V4L2Device &v4l2_device, int width,
                                  int height) {
  assert(textures_.empty());

  const int kNumBuffers = 2;

  // TODO: check VIDEO_QUERYCAP

  // TODO: refactor width/height
  auto may_format = v4l2_device.setFormat(V4L2Device::Format{width, height});
  if (!may_format) {
    return false;
  }
  width = may_format->width;
  height = may_format->height;

  int buffer_count = v4l2_device.requestBuffer(kNumBuffers);
  if (buffer_count < 0) {
    return false;
  }

  VLOG(0) << "buffer_count=" << buffer_count;

  // auto format = getV4L2Format(device_fd);
  // if (!format.has_value()) {
  //   return false;
  // }
  // struct v4l2_buffer buffer;
  // buffer.length = format->fmt.pix_mp.num_planes;
  // buffer.index = buffer_id;
  // buffer.type = type;
  // buffer.memory = memory;

  std::vector<int> buffer_fds = v4l2_device.openDmaBuf(buffer_count);
  if ((int)buffer_fds.size() != buffer_count) {
    return false;
  }

  for (int i = 0; i < buffer_count; ++i) {
    v4l2_device.queueBuffer(i);
  }

  textures_ = bindTextures(egl_display, buffer_fds, width, height);

  if (!v4l2_device.startV4L2stream()) {
    return false;
  }

  return true;
}

void DMABufferTexture::bindTexture(int idx) const {
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, textures_[idx]);
}

void DMABufferTexture::dequeue(V4L2Device &v4l2_device) const {

  // int device_fd = openV4L2Device();
  // if (device_fd < 0)
  //   return;

  // int buffer_count = textures_.size();

  // for (int i = 0; i < buffer_count; ++i) {
  //   dequeueV4L2Buffer(device_fd, i);
  // }
}

void DMABufferTexture::queue(V4L2Device &v4l2_device) const {

  // int device_fd = openV4L2Device();
  // if (device_fd < 0)
  //   return;

  // int buffer_count = textures_.size();

  // for (int i = 0; i < buffer_count; ++i) {
  //   queueV4L2Buffer(device_fd, i);
  // }
}