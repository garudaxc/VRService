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
#include <GLES/gl.h>
#include <unistd.h>


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
    //LOGI("step");
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

        LOGI("fps %.2f", fps);
    }
}



jclass		surfaceClass;
jmethodID	setFrontBufferID;

void init(JNIEnv *env, jobject thiz, jint width, jint height)
{

    if ( env == NULL ) {
        LOGE( "VrSurfaceManager::Init - Invalid jni" );
        return;
    }

    // Determine if the Java Front Buffer IF exists. If not, fall back
    // to using the egl extensions.
    jclass lc = env->FindClass( "android/app/VRSurfaceManager" );
    if ( lc != NULL ) {
        surfaceClass = (jclass)env->NewGlobalRef( lc );
        LOGI( "Found VrSurfaceManager API: %p", surfaceClass );
        env->DeleteLocalRef( lc );
    }

    // Clear NoClassDefFoundError, if thrown
    if ( env->ExceptionOccurred() ) {
        env->ExceptionClear();
        LOGI( "Clearing JNI Exceptions" );
    }

    // Look up the Java Front Buffer IF method IDs
    if ( surfaceClass != NULL ) {
        setFrontBufferID = env->GetStaticMethodID( surfaceClass, "setFrontBuffer", "(IZ)V" );
//        getFrontBufferAddressID = env->GetStaticMethodID( surfaceClass, "getFrontBufferAddress", "(I)I" );
//        getSurfaceBufferAddressID = env->GetStaticMethodID( surfaceClass, "getSurfaceBufferAddress", "(I[II)I" );
//        getClientBufferAddressID = env->GetStaticMethodID( surfaceClass, "getClientBufferAddress", "(I)I" );
    }

    if (setFrontBufferID == NULL) {
        LOGE("can not find jni method setFrontBuffer!");
        return;
    }


    EGLSurface windowSurface = eglGetCurrentSurface(EGL_DRAW);

    LOGI("Calling java method");
    // Use the Java Front Buffer IF
    env->CallStaticVoidMethod(surfaceClass, setFrontBufferID, (int) windowSurface, true);

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
