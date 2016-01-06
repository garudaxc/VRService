/*
* Copyright (C) 2008 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#define LOG_TAG "simplejni"
#include <utils/Log.h>
#include <binder/BinderService.h>
#include <binder/IServiceManager.h>
#include <surfaceflinger/SurfaceComposerClient.h>
#include <surfaceflinger/ISurfaceComposer.h>

#include <stdio.h>
#include <ui/FramebufferNativeWindow.h>

#include "jni.h"
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <unistd.h>


using namespace android;
EGLDisplay display = NULL;
EGLSurface surface = NULL;


#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT uint64_t JNICALL GetTicksNanos();
JNIIMPORT uint32_t JNICALL GetTicksMS();

#ifdef __cplusplus
}
#endif


uint64_t GetTicksNanos()
{
    // Do NOT change because this should be the same as Java's system.nanoTime(),
    // which is what the Choreographer vsync timestamp is based on.
    struct timespec tp;
    const int       status = clock_gettime(CLOCK_MONOTONIC, &tp);

    if (status != 0)
    {
        //OVR_DEBUG_LOG(("clock_gettime status=%i", status));
    }
    const uint64_t result = (uint64_t)tp.tv_sec * (uint64_t)(1000 * 1000 * 1000) + uint64_t(tp.tv_nsec);
    return result;
}

uint32_t GetTicksMS()
{
    return GetTicksNanos() / 1000000;
}


static void TestDraw()
{
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(display, surface);
}



static jint
add(JNIEnv *env, jobject thiz, jint a, jint b) {
    int result = a + b + 55;
    LOGI("%d + %d = %d", a, b, result);

    return result;
}

static void step(JNIEnv *env, jobject thiz)
{
    TestDraw();

    static uint32_t lastTime = GetTicksMS();
    static uint32_t numFrames = 0;

    numFrames++;
    uint32_t time = GetTicksMS();
    if (time - lastTime > 3000) {
        float fps = (numFrames * 1000.f) / (time - lastTime);
        numFrames = 0;
        lastTime = time;

        LOGI("fps %.2f", fps);
    }
}

static void init(JNIEnv *env, jobject thiz, jint width, jint height)
{

    EGLNativeWindowType window = android_createDisplaySurface();
    LOGI("EGLNativeWindowType %p", window);

    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLContext context;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    //ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    LOGI("surface %p", surface);
    LOGI("context %p", context);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGI("Unable to eglMakeCurrent");
    }

    //sp<ISurfaceComposer> surfaceComposer = ComposerService::getComposerService();
    //LOGI("SurfaceFlinger services ISurfaceComposer %p", surfaceComposer.get());

    //surfaceComposer->disableRenderThread();

}

//static const char *classPathName = "com/example/android/simplejni/Native";

static const char *classPathName = "com/example/garuda/myapplication2/Native";


static JNINativeMethod methods[] = {
        {"add", "(II)I", (void*)add },
        {"step", "()V", (void*)step },
        {"init", "(II)V", (void*)init },
};

/*
* Register several native methods for one class.
*/
static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
* Register native methods for all classes we know about.
*
* returns JNI_TRUE on success.
*/
static int registerNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, classPathName,
                               methods, sizeof(methods) / sizeof(methods[0]))) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


// ----------------------------------------------------------------------------

/*
* This is called by the VM when the shared library is first loaded.


typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;

    LOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        LOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

    bail:
    return result;
}
 */