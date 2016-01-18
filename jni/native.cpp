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
#include <stdio.h>
#include "jni.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <unistd.h>
#include <ui/FramebufferNativeWindow.h>
#include <system/window.h>
#include <android/native_window_jni.h>

using namespace android;


EGLDisplay display = NULL;
EGLSurface surface = NULL;


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



void step(JNIEnv *env, jobject thiz)
{
    //ALOGI("step");
    return;

    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(display, surface);

    static uint32_t lastTime = GetTicksMS();
    static uint32_t numFrames = 0;

    numFrames++;
    uint32_t time = GetTicksMS();
    if (time - lastTime > 3000) {
        float fps = (numFrames * 1000.f) / (time - lastTime);
        numFrames = 0;
        lastTime = time;

        ALOGI("fps %.2f", fps);
    }
}

jobject nativeCreateSurface(JNIEnv *env, jobject thiz)
{
    EGLNativeWindowType window = android_createDisplaySurface();
    ALOGI("create EGLNativeWindowType %p", window);

    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RECORDABLE_ANDROID, EGL_TRUE,
            EGL_FRAMEBUFFER_TARGET_ANDROID, EGL_TRUE,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;


    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    ALOGI("numconfig %d", numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ALOGI("format %d", format);


    EGLSurface framebufferSurface = eglCreateWindowSurface(display, config, window, NULL);

    EGLint mWidth, mHeight;
    eglQuerySurface(display, framebufferSurface, EGL_WIDTH,  &mWidth);
    eglQuerySurface(display, framebufferSurface, EGL_HEIGHT, &mHeight);
    ALOGI("framebufferSurface %d x %d", mWidth, mHeight);

    //int32_t bSetWindowBuffer = ANativeWindow_setBuffersGeometry(nativeWin, mWidth, mHeight, format);
    //ALOGI("ANativeWindow_setBuffersGeometry res %d", bSetWindowBuffer);


    jclass clazz = env->FindClass("com/google/android/gles_jni/EGLSurfaceImpl");
    ALOGI("found class %p", clazz);
    if (clazz == NULL) {
        return NULL;
    }

    jmethodID construct = env->GetMethodID(clazz,"<init>","(I)V");
    ALOGI("construct method %p", construct);

    if (construct == NULL)
    {
        return NULL;
    }

    jobject surface = env->NewObject(clazz,construct, (jint)framebufferSurface);
    ALOGI("new object %p", surface);

    return surface;
}



jclass		surfaceClass;
jmethodID	setFrontBufferID;


EGLAPI EGLint EGLAPIENTRY eglExchangeSurfaceFTVR(EGLSurface sufa, EGLSurface sufb);

void init(JNIEnv *env, jobject thiz, jint width, jint height, jobject surface)
{
    ANativeWindow* nativeWin = ANativeWindow_fromSurface(env, surface);
    ALOGI("native window %p", nativeWin);

    EGLNativeWindowType window = android_createDisplaySurface();
    ALOGI("create EGLNativeWindowType %p", window);

    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_RECORDABLE_ANDROID, EGL_TRUE,
            EGL_FRAMEBUFFER_TARGET_ANDROID, EGL_TRUE,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;


    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    ALOGI("numconfig %d", numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ALOGI("format %d", format);


    EGLSurface framebufferSurface = eglCreateWindowSurface(display, config, window, NULL);

    EGLint mWidth, mHeight;
    eglQuerySurface(display, framebufferSurface, EGL_WIDTH,  &mWidth);
    eglQuerySurface(display, framebufferSurface, EGL_HEIGHT, &mHeight);
    ALOGI("framebufferSurface %d x %d", mWidth, mHeight);

    int32_t bSetWindowBuffer = ANativeWindow_setBuffersGeometry(nativeWin, mWidth, mHeight, format);
    ALOGI("ANativeWindow_setBuffersGeometry res %d", bSetWindowBuffer);

    EGLSurface windowSurface = eglGetCurrentSurface( EGL_DRAW );
    eglExchangeSurfaceFTVR(windowSurface, framebufferSurface);

    //gOrigSurface = framebufferSurface;

    //EGLBoolean bRes = eglDestroySurface(display, surface);
    //ALOGI("surface destroyed : %d", bRes ? 1 : 0);

    EGLContext context = eglGetCurrentContext();
    if (eglMakeCurrent(display, windowSurface, windowSurface, context) == EGL_FALSE) {
        ALOGI("Unable to eglMakeCurrent");
    }

    glClearColor(0.0f, 1.0f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    return;













    if ( env == NULL ) {
        ALOGE( "VrSurfaceManager::Init - Invalid jni" );
        return;
    }

    // Determine if the Java Front Buffer IF exists. If not, fall back
    // to using the egl extensions.
    jclass lc = env->FindClass( "android/app/VRSurfaceManager" );
    if ( lc != NULL ) {
        surfaceClass = (jclass)env->NewGlobalRef( lc );
        ALOGI( "Found VrSurfaceManager API: %p", surfaceClass );
        env->DeleteLocalRef( lc );
    }

    // Clear NoClassDefFoundError, if thrown
    if ( env->ExceptionOccurred() ) {
        env->ExceptionClear();
        ALOGI( "Clearing JNI Exceptions" );
    }

    // Look up the Java Front Buffer IF method IDs
    if ( surfaceClass != NULL ) {
        setFrontBufferID = env->GetStaticMethodID( surfaceClass, "setFrontBuffer", "(IZ)V" );
//        getFrontBufferAddressID = env->GetStaticMethodID( surfaceClass, "getFrontBufferAddress", "(I)I" );
//        getSurfaceBufferAddressID = env->GetStaticMethodID( surfaceClass, "getSurfaceBufferAddress", "(I[II)I" );
//        getClientBufferAddressID = env->GetStaticMethodID( surfaceClass, "getClientBufferAddress", "(I)I" );
    }

    if (setFrontBufferID == NULL) {
        ALOGE("can not find jni method setFrontBuffer!");
        return;
    }


//    EGLSurface windowSurface = eglGetCurrentSurface(EGL_DRAW);
//
//    ALOGI("Calling java method");
//    // Use the Java Front Buffer IF
//    env->CallStaticVoidMethod(surfaceClass, setFrontBufferID, (int) windowSurface, true);

}

void __pause()
{
    //sp<ISurfaceComposer> surfaceComposer = ComposerService::getComposerService();
    //surfaceComposer->setEnable(true);
}

jobject glViewObject = NULL;
void surfaceJni(JNIEnv *env, jobject thiz)
{
//    glViewObject = thiz;
//    env->GetFieldID(thiz, "")
}

//static const char *classPathName = "com/example/android/simplejni/Native";

static const char *classPathName = "com/example/garuda/myapplication/Native";


static JNINativeMethod methods[] = {
        {"step", "()V", (void*)step },
        {"init", "(IILjava/lang/Object;)V", (void*)init },
        {"nativeCreateSurface", "()Ljava/lang/Object;", (void*)nativeCreateSurface },
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
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'", className);
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


// * This is called by the VM when the shared library is first loaded.


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

    ALOGI("JNI_OnLoad");

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }
    env = uenv.env;

    if (registerNatives(env) != JNI_TRUE) {
        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

    bail:
    return result;
}
