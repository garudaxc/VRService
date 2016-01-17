
#define LOG_TAG "VRSurfaceManager"

#include <cutils/log.h>
#include <jni.h>
#include <stdio.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES/gl.h>
#include <unistd.h>

#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <ui/FramebufferNativeWindow.h>


using namespace android;

EGLAPI EGLint EGLAPIENTRY eglExchangeSurfaceFTVR(EGLSurface sufa, EGLSurface sufb);


EGLSurface gOrigSurface = NULL;

static void SetFrontBuffer(JNIEnv *env, jobject thiz, jint surface, jboolean bSet)
{
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLSurface windowSurface = (EGLSurface)surface;
    ALOGI("native SetFrontBuffer %p", windowSurface);

    if (gOrigSurface != NULL) {
        ALOGE("gOrigSurface not NULL, front buffer may already seted!");
        return;
    }

    int mWidth, mHeight;
    eglQuerySurface(display, windowSurface, EGL_WIDTH,  &mWidth);
    eglQuerySurface(display, windowSurface, EGL_HEIGHT, &mHeight);
    ALOGI("windowsurface %d x %d", mWidth, mHeight);

    EGLContext context = eglGetCurrentContext();
    ALOGI("current context %p", context);


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


    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig tdhat matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    ALOGI("numconfigs %d", numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    //ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    EGLSurface framebufferSurface = eglCreateWindowSurface(display, config, window, NULL);

    eglQuerySurface(display, framebufferSurface, EGL_WIDTH,  &mWidth);
    eglQuerySurface(display, framebufferSurface, EGL_HEIGHT, &mHeight);
    ALOGI("framebuffer surface %d x %d", mWidth, mHeight);
    eglExchangeSurfaceFTVR(windowSurface, framebufferSurface);

    eglQuerySurface(display, windowSurface, EGL_WIDTH,  &mWidth);
    eglQuerySurface(display, windowSurface, EGL_HEIGHT, &mHeight);
    ALOGI("windowsurface %d x %d", mWidth, mHeight);

    gOrigSurface = framebufferSurface;

    //EGLBoolean bRes = eglDestroySurface(display, surface);
    //ALOGI("surface destroyed : %d", bRes ? 1 : 0);

    if (eglMakeCurrent(display, windowSurface, windowSurface, context) == EGL_FALSE) {
        ALOGI("Unable to eglMakeCurrent");
    }

    glClearColor(1.0f, 0.0f, 0.0f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);


    //sp<ISurfaceComposer> surfaceComposer = ComposerService::getComposerService();
    //ALOGI("SurfaceFlinger services ISurfaceComposer %p", surfaceComposer.get());
    //surfaceComposer->setEnable(false);

}










//----------------------------------------------------------



static const char *classPathName = "android/app/VRSurfaceManager";
static JNINativeMethod methods[] = {
        {"nativeSetFrontBuffer", "(IZ)V", (void*)SetFrontBuffer },
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

    ALOGI("VRSurfaceManager JNI_OnLoad");

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
