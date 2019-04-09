/*
 * Copyright (c) 2018-present, lmyooyo@gmail.com.
 *
 * This source code is licensed under the GPL license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef HARDWAREVIDEOCODEC_EGL_H
#define HARDWAREVIDEOCODEC_EGL_H

#include <EGL/egl.h>
#include "Object.h"

const int CONFIG_DEFAULT[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                              EGL_RED_SIZE, 8,
                              EGL_GREEN_SIZE, 8,
                              EGL_BLUE_SIZE, 8,
                              EGL_ALPHA_SIZE, 8,
                              EGL_DEPTH_SIZE, 0,
                              EGL_STENCIL_SIZE, 0,
                              EGL_SAMPLES, 4,
                              EGL_NONE};

class Egl : public Object {
public:
    Egl();

    Egl(Egl *context);

    Egl(ANativeWindow *win);

    Egl(Egl *context, ANativeWindow *win);

    virtual ~Egl();

    void makeCurrent();

    void swapBuffers();

    int width();

    int height();

private:
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLConfig eglConfig = nullptr;
    EGLContext eglContext = EGL_NO_CONTEXT;
    EGLSurface eglSurface = EGL_NO_SURFACE;

    void init(Egl *context, ANativeWindow *win);

    EGLDisplay createDisplay(EGLNativeDisplayType display_id);

    EGLConfig createConfig(const int *configSpec);

    EGLContext createContext(EGLContext context);

    EGLSurface createPbufferSurface();

    EGLSurface createWindowSurface(ANativeWindow *win);

    EGLint getParams(EGLint attribute);
};


#endif //HARDWAREVIDEOCODEC_EGL_H
