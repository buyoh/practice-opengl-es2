#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "base/logging.h"
#include "window/awindow_x11.h"

namespace {

class DisplayX11 {
public:
  DisplayX11();
  ~DisplayX11();

  Display *getXDisplay() { return xdisplay_; }

private:
  Display *xdisplay_ = nullptr;
} g_display;

DisplayX11::DisplayX11() {
  //
  xdisplay_ = XOpenDisplay(nullptr);
  if (!xdisplay_) {
    LOG_E << "XOpenDisplay";
  }
}

DisplayX11::~DisplayX11() {
  if (xdisplay_) {
    XCloseDisplay(xdisplay_);
    xdisplay_ = nullptr;
  }
}

} // namespace

//

AWindowX11::AWindowX11() {}

AWindowX11::~AWindowX11() {
  Display *display = g_display.getXDisplay();

  if (display == nullptr) {
    return;
  }

  if (window_)
    XDestroyWindow(display, window_);
}

bool AWindowX11::initialize() {

  Display *display = g_display.getXDisplay();

  if (display == nullptr) {
    return false;
  }

  window_ = XCreateSimpleWindow(display, DefaultRootWindow(display), 100, 100,
                                1024, 768, 1, BlackPixel(display, 0),
                                WhitePixel(display, 0));

  XMapWindow(display, window_);

  return true;
}

void *AWindowX11::getNativeDisplay() const { return g_display.getXDisplay(); }

void *AWindowX11::getNativeWindow() const {
  return reinterpret_cast<void *>(window_);
}
