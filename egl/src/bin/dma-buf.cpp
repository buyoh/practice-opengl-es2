
//
#include <iostream>
//
#include <fcntl.h>
#include <libdrm/drm_fourcc.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
//
#include <EGL/egl.h>
#include <GLES2/gl2.h>
//
#include <EGL/eglext.h>
#include <GL/gl.h>
// GL_TEXTURE_EXTERNAL_OES in <GLES2/gl2ext.h>
#include <GLES2/gl2ext.h>

namespace {

// <EGL/eglext.h>
PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
// <GL/gl.h>
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

#define LOOKUP_EXT_FUNCTION(F)                                                 \
  F = reinterpret_cast<decltype(F)>(eglGetProcAddress(#F));                    \
  if (!F) {                                                                    \
    std::clog << "eglGetProcAddress(" #F ")\n";                                \
    abort();                                                                   \
  }

void initializeEglExtensionFunctions() {
  LOOKUP_EXT_FUNCTION(eglCreateImageKHR)
  LOOKUP_EXT_FUNCTION(eglDestroyImageKHR)
  LOOKUP_EXT_FUNCTION(glEGLImageTargetTexture2DOES)
}

} // namespace

const int MAX_BUFFERS = 4;

void initDnaBuf() {
  const char *filepath = "/dev/video0";

  int fd = open(filepath, O_RDWR | O_NONBLOCK | O_CLOEXEC);

  // v4l2_fd_open(fd, V4L2_DISABLE_CONVERSION)  // ????

  int fds[MAX_BUFFERS];
  const int num_buffers = MAX_BUFFERS;
  for (unsigned int i = 0; i < num_buffers; ++i) {
    struct v4l2_exportbuffer exbuf;
    exbuf.index = i;
    exbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_EXPBUF, &exbuf);

    fds[i] = exbuf.fd; // get fd via ioctl
  }

  GLuint textures[MAX_BUFFERS];
  ::glGenTextures(num_buffers, textures);
  for (auto i = 0u; i < num_buffers; ++i) {
    EGLint attrs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE,
                      //
                      EGL_WIDTH, width,
                      //
                      EGL_HEIGHT, height,
                      //
                      EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ABGR8888,
                      //
                      EGL_DMA_BUF_PLANE0_FD_EXT, fds[i],
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
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
  }
}