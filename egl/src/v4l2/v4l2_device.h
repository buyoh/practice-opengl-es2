#ifndef SRC_V4L2_V4L2_DEVICE_H_
#define SRC_V4L2_V4L2_DEVICE_H_

#include <optional>
#include <vector>

class V4L2Device {
 public:
  struct Format {
    int width;
    int height;
  };
  struct CaptureParameter {
    std::optional<double> time_per_frame;
  };

  V4L2Device() = default;
  ~V4L2Device();

  bool open(const char* filepath);
  void close();

  bool isInitialized() const noexcept { return device_fd_ >= 0; }

  std::optional<Format> getFormatVideoCapture();
  std::optional<Format> setFormatVideoCapture(const Format& format);
  std::optional<CaptureParameter> getParameterVideoCapture();

  // return -1 : error
  // return >= 0 : success (true buffer_count)
  int requestBuffer(int buffer_count);
  bool queryBuffer(int buffer_index);
  bool queueBuffer(int buffer_index, int fd);
  bool dequeueBuffer(int buffer_index, int fd);

  std::vector<int> openDmaBuf(int num_buffer);

  bool startV4L2stream();
  bool stopV4L2stream();

 private:
  int device_fd_ = -1;
};

#endif  // SRC_V4L2_V4L2_DEVICE_H_