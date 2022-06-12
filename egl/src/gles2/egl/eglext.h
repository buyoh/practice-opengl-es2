#ifndef SRC_GLES2_EGL_EGLEXT_H_
#define SRC_GLES2_EGL_EGLEXT_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>

// <EGL/eglext.h>
extern PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
extern PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
// <GL/gl.h>
extern PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;

#endif // SRC_GLES2_EGL_EGLEXT_H_