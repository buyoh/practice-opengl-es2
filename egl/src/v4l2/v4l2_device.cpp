//
#include <cassert>
#include <optional>
#include <vector>
//
#include <errno.h>
#include <fcntl.h>
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/ioctl.h>
//
#include "base/logging.h"
#include "gles2/egl/dma_buffer_texture.h"
#include "gles2/egl/eglext.h"
//
#include "v4l2_device.h"

V4L2Device::~V4L2Device() {
  //
  close();
}

bool V4L2Device::open(const char* filepath) {
  int device_fd = ::open(filepath, O_RDWR);  // O_NONBLOCK | O_CLOEXEC
  if (device_fd <= -1) {
    LOG_E << "open `" << filepath << "` failed: " << strerror(errno);
    return false;
  }
  device_fd_ = device_fd;

  // v4l2_fd_open(fd, V4L2_DISABLE_CONVERSION)  // ????

  return true;
}
void V4L2Device::close() {
  if (device_fd_ > 0)
    ::close(device_fd_);
  device_fd_ = -1;
}

std::optional<V4L2Device::Format> V4L2Device::getFormatVideoCapture() {
  assert(isInitialized());

  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd_, VIDIOC_G_FMT, &fmt) < 0) {
    LOG_E << "VIDIOC_G_FMT: ioctl failed: " << strerror(errno);
    return std::optional<V4L2Device::Format>();
  }

  VLOG(0) << "width=" << fmt.fmt.pix.width << " height=" << fmt.fmt.pix.height;

  V4L2Device::Format format;
  format.width = fmt.fmt.pix.width;
  format.height = fmt.fmt.pix.height;

  return format;
}

std::optional<V4L2Device::Format> V4L2Device::setFormatVideoCapture(
    const Format& format) {
  assert(isInitialized());

  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.fmt.pix.width = format.width;
  fmt.fmt.pix.height = format.height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  // format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV32;
  // format.fmt.pix.field = V4L2_FIELD_INTERLACED;
  // format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd_, VIDIOC_S_FMT, &fmt) < 0) {
    LOG_E << "VIDIOC_S_FMT: ioctl failed: " << strerror(errno);
    return std::optional<V4L2Device::Format>();
  }

  VLOG(0) << "width=" << fmt.fmt.pix.width << " height=" << fmt.fmt.pix.height;

  assert(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV);
  // assert(format.fmt.pix.width == width);
  // assert(format.fmt.pix.height == height);
  // TODO: feedback width / height

  V4L2Device::Format updated_format;
  updated_format.width = fmt.fmt.pix.width;
  updated_format.height = fmt.fmt.pix.height;

  return updated_format;
}

std::optional<V4L2Device::CaptureParameter>
V4L2Device::getParameterVideoCapture() {
  assert(isInitialized());

  struct v4l2_streamparm prm;
  memset(&prm, 0, sizeof(prm));
  prm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd_, VIDIOC_G_PARM, &prm) < 0) {
    LOG_E << "VIDIOC_G_PARM: ioctl failed: " << strerror(errno);
    return std::optional<V4L2Device::CaptureParameter>();
  }

  CaptureParameter capture_paramter;

  bool is_supported_timeperframe =
      prm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME;
  // bool highquality = prm.parm.capture.capturemode & V4L2_MODE_HIGHQUALITY;
  if (is_supported_timeperframe) {
    double n = prm.parm.capture.timeperframe.numerator;
    double d = prm.parm.capture.timeperframe.denominator;
    if (d != 0)
      capture_paramter.time_per_frame = n / d;

    VLOG(0) << "timeperframe is supported:" << n << "/" << d;
  } else {
    VLOG(0) << "timeperframe is not supported";
  }

  return capture_paramter;
}

int V4L2Device::requestBuffer(int buffer_count) {
  assert(isInitialized());

  struct v4l2_requestbuffers request;

  memset(&request, 0, sizeof(request));
  request.count = buffer_count;
  // request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // request.memory = V4L2_MEMORY_DMABUF;
  request.memory = V4L2_MEMORY_MMAP;

  if (ioctl(device_fd_, VIDIOC_REQBUFS, &request) < 0) {
    LOG_E << "VIDIOC_REQBUFS: ioctl failed: " << strerror(errno);
    return -1;
  }

  return request.count;
}

bool V4L2Device::queryBuffer(int buffer_index) {
  assert(isInitialized());
  struct v4l2_buffer buffer;
  memset(&buffer, 0, sizeof(buffer));

  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_MMAP;
  buffer.index = buffer_index;

  if (ioctl(device_fd_, VIDIOC_QUERYBUF, &buffer) < 0) {
    LOG_E << "VIDIOC_QUERYBUF: ioctl failed: " << strerror(errno);
    return false;
  }

#if 0
  void *ptr = mmap(NULL, buffer.length, PROT_READ, MAP_SHARED, device_fd,
                   buffer.m.offset);
  if (mmap_p[i] == MAP_FAILED) {
    return false;
  }
  auto length = buffer.length;
#endif
  return true;
}

bool V4L2Device::queueBuffer(int buffer_index, int fd) {
  assert(isInitialized());

  struct v4l2_buffer buffer;
  memset(&buffer, 0, sizeof(buffer));

  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // buffer.memory = V4L2_MEMORY_DMABUF;
  buffer.memory = V4L2_MEMORY_MMAP;
  buffer.index = buffer_index;
  buffer.m.fd = 0;

  if (ioctl(device_fd_, VIDIOC_QBUF, &buffer) < 0) {
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

bool V4L2Device::dequeueBuffer(int buffer_index, int fd) {
  assert(isInitialized());

  struct v4l2_buffer buffer;
  memset(&buffer, 0, sizeof(buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // buffer.memory = V4L2_MEMORY_DMABUF;
  buffer.memory = V4L2_MEMORY_MMAP;
  buffer.index = buffer_index;
  buffer.m.fd = fd;

  if (ioctl(device_fd_, VIDIOC_DQBUF, &buffer) < 0) {
    LOG_E << "VIDIOC_DQBUF: ioctl failed: " << strerror(errno);
    return false;
  }

  return true;
}

std::vector<int> V4L2Device::openDmaBuf(int num_buffer) {
  assert(isInitialized());

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
    expbuf.flags = O_CLOEXEC;  // ??
    if (ioctl(device_fd_, VIDIOC_EXPBUF, &expbuf) < 0) {
      LOG_E << "VIDIOC_EXPBUF ioctl failed(i=" << i << "): " << strerror(errno);
      return std::vector<int>();
    }

    fds[i] = expbuf.fd;  // get fd via ioctl
  }
  return fds;
}

bool V4L2Device::startV4L2stream() {
  assert(isInitialized());

  int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd_, VIDIOC_STREAMON, &buffer_type) < 0) {
    LOG_E << "VIDIOC_STREAMON: ioctl failed: " << strerror(errno);
    return false;
  }
  return true;
}

bool V4L2Device::stopV4L2stream() {
  assert(isInitialized());

  int buffer_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (ioctl(device_fd_, VIDIOC_STREAMOFF, &buffer_type) < 0) {
    LOG_E << "VIDIOC_STREAMON: ioctl failed: " << strerror(errno);
    return false;
  }
  return true;
}
