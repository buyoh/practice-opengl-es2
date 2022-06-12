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

  V4L2Device() = default;
  ~V4L2Device();

  bool open(const char* filepath);
  void close();

  bool isInitialized() const noexcept { return device_fd_ >= 0; }

  std::optional<Format> getFormat();
  std::optional<Format> setFormat(const Format& format);

  // return -1 : error
  // return >= 0 : success (true buffer_count)
  int requestBuffer(int buffer_count);
  bool queryBuffer(int buffer_index);
  bool queueBuffer(int buffer_index);
  bool dequeueBuffer(int buffer_index);

  std::vector<int> openDmaBuf(int num_buffer);

  bool startV4L2stream();

 private:
  int device_fd_ = -1;
};

#endif  // SRC_V4L2_V4L2_DEVICE_H_