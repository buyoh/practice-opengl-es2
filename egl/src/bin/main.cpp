#include <iostream>

#include "app/app.h"
#include "egl/aegl.h"
#include "window/awindow_x11.h"

int main(int argc, char *argv[]) {

  AWindowX11 window_x11;
  if (!window_x11.initialize()) {
    return 2;
  }

  AEgl egl;
  if (!egl.initialize(window_x11.getNativeDisplay(),
                      window_x11.getNativeWindow(), true)) {
    return 2;
  }

  App::mainloop(egl.getDisplay(), egl.getSurface());

  std::cout << "quit" << std::endl;

  return 0;
}
