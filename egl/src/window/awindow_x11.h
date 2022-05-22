#ifndef EGL_SRC_WINDOW_AWINDOW_X11_H_
#define EGL_SRC_WINDOW_AWINDOW_X11_H_

#include <X11/X.h>

#include "./awindow.h"

class AWindowX11 : public AWindow {
public:
  AWindowX11();
  ~AWindowX11();
  bool initialize() override;

  void *getNativeDisplay() override;
  void *getNativeWindow() override;

private:
  Window window_ = 0;
};

#endif // EGL_SRC_WINDOW_AWINDOW_X11_H_
