//
#include <cassert>
#include <iostream>
#include <libdrm/drm.h>
#include <optional>
#include <vector>
//
#include <errno.h>
#include <fcntl.h>
#include <libdrm/drm_fourcc.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/ioctl.h>
//
#include <EGL/egl.h>
#include <GLES2/gl2.h>
//
#include <EGL/eglext.h>
#include <GL/gl.h>
// GL_TEXTURE_EXTERNAL_OES in <GLES2/gl2ext.h>
#include <GLES2/gl2ext.h>

#include "base/logging.h"
#include "gles2/utils.h"

#include "gles2/egl/dma_buffer_texture.h"

// https://manual.atmark-techno.com/armadillo-810/armadillo-810_product_manual_ja-1.3.0/ch14.html

namespace {

// <EGL/eglext.h>
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
// <GL/gl.h>
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

// TODO: lock
int g_device_fd = -1;

#define LOOKUP_EXT_FUNCTION(F)                                                 \
  F = reinterpret_cast<decltype(F)>(eglGetProcAddress(#F));                    \
  if (!F) {                                                                    \
    std::clog << "eglGetProcAddress(" #F ")\n";                                \
    abort();                                                                   \
  }

void initializeEglExtensionFunctions() {
  std::clog << "initializeEglExtensionFunctions\n";
  LOOKUP_EXT_FUNCTION(eglCreateImageKHR)
  LOOKUP_EXT_FUNCTION(eglDestroyImageKHR)
  LOOKUP_EXT_FUNCTION(glEGLImageTargetTexture2DOES)
}

class Initializer {
public:
  // TODO: refactoring
  Initializer() { initializeEglExtensionFunctions(); }
} g_initializer;

//
// TODO: refactoring... separete V4L2

int openV4L2Device() {
  // TODO: close
  if (g_device_fd >= 0)
    return g_device_fd;

  const char *filepath = "/dev/video0";

  int g_device_fd = open(filepath, O_RDWR); // O_NONBLOCK | O_CLOEXEC
  if (g_device_fd <= -1) {
    LOG_E << "open `" << filepath << "` failed: " << strerror(errno);
    return -1;
  }

  // v4l2_fd_open(fd, V4L2_DISABLE_CONVERSION)  // ????

  return g_device_fd;
}

std::optional<v4l2_format> getV4L2Format(int device_fd) {
  struct v4l2_format format;
  memset(&format, 0, sizeof(format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd, VIDIOC_G_FMT, &format) < 0) {
    LOG_E << "VIDIOC_G_FMT: ioctl failed: " << strerror(errno);
    return std::optional<v4l2_format>();
  }

  VLOG(0) << "width=" << format.fmt.pix.width
          << " height=" << format.fmt.pix.height;

  return format;
}

bool setV4L2Format(int device_fd, int width, int height) {
  // TODO: bad interface
  struct v4l2_format format;
  memset(&format, 0, sizeof(format));
  format.fmt.pix.width = width;
  format.fmt.pix.height = height;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  // format.fmt.pix.field = V4L2_FIELD_INTERLACED;
  // format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd, VIDIOC_S_FMT, &format) < 0) {
    LOG_E << "VIDIOC_S_FMT: ioctl failed: " << strerror(errno);
    return false;
  }

  VLOG(0) << "width=" << format.fmt.pix.width
          << " height=" << format.fmt.pix.height;

  assert(format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV);
  assert(format.fmt.pix.width == width);
  assert(format.fmt.pix.height == height);
  // TODO: feedback width / height

  return true;
}

#if 0
bool queryV4L2Buffer(int device_fd, int buffer_index) {
  struct v4l2_buffer buffer;
  memset(&buffer, 0, sizeof(buffer));

  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;
  buffer.index = buffer_index;

  if (ioctl(device_fd, VIDIOC_QUERYBUF, &buffer) < 0) {
    LOG_E << "VIDIOC_QUERYBUF: ioctl failed: " << strerror(errno);
    return false;
  }

  void *ptr = mmap(NULL, buffer.length, PROT_READ, MAP_SHARED, device_fd,
                   buffer.m.offset);
  if (mmap_p[i] == MAP_FAILED) {
    return false;
  }
  auto length = buffer.length;

  // TODO:
  return true;
}
#endif

int requestV4L2Buffer(int device_fd, int buffer_count) {
  struct v4l2_requestbuffers request;

  memset(&request, 0, sizeof(request));
  request.count = buffer_count;
  // request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // request.memory = V4L2_MEMORY_DMABUF;
  request.memory = V4L2_MEMORY_MMAP;

  if (ioctl(device_fd, VIDIOC_REQBUFS, &request) < 0) {
    LOG_E << "VIDIOC_REQBUFS: ioctl failed: " << strerror(errno);
    return -1;
  }

  return request.count;
}

std::vector<int> openV4L2Buffer(int device_fd, int num_buffer) {
  // TODO: rename
  // const int index = 0;
  // TODO: close
  std::vector<int> fds(num_buffer);
  for (int i = 0; i < num_buffer; ++i) {
    struct v4l2_exportbuffer expbuf;
    memset(&expbuf, 0, sizeof(expbuf));
    expbuf.index = i;
    expbuf.plane = 0;
    expbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    expbuf.flags = O_CLOEXEC; // ??
    if (ioctl(device_fd, VIDIOC_EXPBUF, &expbuf) < 0) {
      LOG_E << "VIDIOC_EXPBUF ioctl failed(i=" << i << "): " << strerror(errno);
      return std::vector<int>();
    }

    fds[i] = expbuf.fd; // get fd via ioctl
  }
  return fds;
}

bool startV4L2stream(int device_fd) {
  int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd, VIDIOC_STREAMON, &buffer_type) < 0) {
    LOG_E << "VIDIOC_STREAMON: ioctl failed: " << strerror(errno);
    return false;
  }
  return true;
}

bool queueV4L2Buffer(int device_fd, int buffer_index) {
  struct v4l2_buffer buffer;
  memset(&buffer, 0, sizeof(buffer));

  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // buffer.memory = V4L2_MEMORY_DMABUF;
  buffer.memory = V4L2_MEMORY_MMAP;
  buffer.index = buffer_index;

  if (ioctl(device_fd, VIDIOC_QBUF, &buffer) < 0) {
    LOG_E << "VIDIOC_QBUF: ioctl failed: " << strerror(errno);
    return false;
  }

  // void *ptr = mmap(NULL, buffer.length, PROT_READ, MAP_SHARED, device_fd,
  //                  buffer.m.offset);
  // if (mmap_p[i] == MAP_FAILED) {
  //   return false;
  // }
  // auto length = buffer.length;

  // TODO:
  return true;
}

bool dequeueV4L2Buffer(int device_fd, int buffer_index) {
  struct v4l2_buffer buffer;
  memset(&buffer, 0, sizeof(buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // buffer.memory = V4L2_MEMORY_DMABUF;
  buffer.memory = V4L2_MEMORY_MMAP;
  buffer.index = buffer_index;

  if (ioctl(device_fd, VIDIOC_DQBUF, &buffer) < 0) {
    LOG_E << "VIDIOC_DQBUF: ioctl failed: " << strerror(errno);
    return false;
  }

  return true;
}

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
                      EGL_DMA_BUF_PLANE0_PITCH_EXT, width * 4,
                      //
                      EGL_NONE};

    EGLImageKHR image = eglCreateImageKHR(egl_display, EGL_NO_CONTEXT,
                                          EGL_LINUX_DMA_BUF_EXT, NULL, attrs);

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

bool DMABufferTexture::initialize(EGLDisplay egl_display, int width,
                                  int height) {
  assert(textures_.empty());

  const int kNumBuffers = 1;

  int device_fd = openV4L2Device();
  if (device_fd < 0)
    return false;

  // TODO: check VIDEO_QUERYCAP

  // if (!setV4L2Format(device_fd, width, height)) {
  //   return false;
  // }
  setV4L2Format(device_fd, width, height);
  getV4L2Format(device_fd);

  int buffer_count = requestV4L2Buffer(device_fd, kNumBuffers);
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

  std::vector<int> buffer_fds = openV4L2Buffer(device_fd, buffer_count);
  if ((int)buffer_fds.size() != buffer_count) {
    return false;
  }

  for (int i = 0; i < buffer_count; ++i) {
    queueV4L2Buffer(device_fd, i);
  }

  textures_ = bindTextures(egl_display, buffer_fds, width, height);

  if (!startV4L2stream(device_fd)) {
    return false;
  }

  return true;
}

void DMABufferTexture::bindTexture(int idx) const {
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, textures_[idx]);
}

void DMABufferTexture::dequeue() const {

  int device_fd = openV4L2Device();
  if (device_fd < 0)
    return;

  int buffer_count = textures_.size();

  for (int i = 0; i < buffer_count; ++i) {
    dequeueV4L2Buffer(device_fd, i);
  }
}

void DMABufferTexture::queue() const {

  int device_fd = openV4L2Device();
  if (device_fd < 0)
    return;

  int buffer_count = textures_.size();

  for (int i = 0; i < buffer_count; ++i) {
    queueV4L2Buffer(device_fd, i);
  }
}