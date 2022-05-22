#ifndef EGL_SRC_WINDOW_AWINDOW_H_
#define EGL_SRC_WINDOW_AWINDOW_H_

class AWindow {
public:
  AWindow() = default;
  virtual bool initialize() = 0;

  // note: 64bit only
  virtual void *getNativeDisplay() = 0;
  virtual void *getNativeWindow() = 0;

private:
};

#endif // EGL_SRC_WINDOW_AWINDOW_H_
