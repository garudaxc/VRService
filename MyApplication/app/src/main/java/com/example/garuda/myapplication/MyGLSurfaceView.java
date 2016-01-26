/**
 * Created by garuda on 12/14/15.
 */

package com.example.garuda.myapplication;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.Surface;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.*;

class BasicGLSurfaceView extends GLSurfaceView {
    static String TAG = "BasicGLSurfaceView";

    class MyConfigChooser implements EGLConfigChooser
    {

        @Override
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {


            int[] configSpec = new int[] {
                    EGL10.EGL_RENDERABLE_TYPE, 4,
                    EGL10.EGL_RED_SIZE, 8,
                    EGL10.EGL_GREEN_SIZE, 8,
                    EGL10.EGL_BLUE_SIZE, 8,
                    EGL10.EGL_ALPHA_SIZE, 8,
                    EGL10.EGL_DEPTH_SIZE, 0,
                    EGL10.EGL_STENCIL_SIZE, 0,
                    //EGL10.EGL_RECORDABLE_ANDROID, EGL_TRUE,
                    //EGL10.EGL_FRAMEBUFFER_TARGET_ANDROID, EGL_TRUE,
                    EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT | EGL10.EGL_PBUFFER_BIT,
                    EGL10.EGL_NONE
            };

            int[] num_config = new int[1];
            if (!egl.eglChooseConfig(display, configSpec, null, 0,
                    num_config)) {
                throw new IllegalArgumentException("eglChooseConfig failed");
            }

            int numConfigs = num_config[0];

            if (numConfigs <= 0) {
                throw new IllegalArgumentException(
                        "No configs match configSpec");
            }

            Log.i(TAG, "chooseConfig: numconfigs " + numConfigs);

            EGLConfig[] configs = new EGLConfig[numConfigs];
            if (!egl.eglChooseConfig(display, configSpec, configs, numConfigs,
                    num_config)) {
                throw new IllegalArgumentException("eglChooseConfig#2 failed");
            }

            return configs[0];
        }
    }

    class MySurfaceFactory implements EGLWindowSurfaceFactory
    {

        @Override
        public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display, EGLConfig config, Object nativeWindow) {
            Object sur = Native.nativeCreateSurface();

            Log.i(TAG, "BasicGLSurfaceView: " + sur);
            if (sur instanceof EGLSurface)
            {
                return (EGLSurface)sur;
            }

            Log.e(TAG, "createWindowSurface: sur not instance of EGLSurface");
            return null;
        }

        @Override
        public void destroySurface(EGL10 egl, EGLDisplay display, EGLSurface surface) {

        }
    }


    public BasicGLSurfaceView(Context context) {
        super(context);
        setEGLContextClientVersion(2);

        //setEGLWindowSurfaceFactory(new MySurfaceFactory());

        setEGLConfigChooser(new MyConfigChooser());

        setRenderer(new Renderer(this));



        //setEGLConfigChooser(8, 8, 8, 0, 0, 0);

//        Class<?> demo=null;
//        try{
//            demo=Class.forName("android.app.VRSurfaceManager");
//        }catch (Exception e) {
//            e.printStackTrace();
//        }
//
//        Object o = null;
//        try {
//            o = demo.newInstance();
//        } catch (InstantiationException e) {
//            // TODO Auto-generated catch block
//            e.printStackTrace();
//        } catch (IllegalAccessException e) {
//            // TODO Auto-generated catch block
//            e.printStackTrace();
//        }
//
//
////        Method[] methods = demo.getMethods();
////        for (int i = 0; i < methods.length; i++)
////        {
////            Method m = methods[i];
////            Log.i(TAG, String.format("method %d %s", i, m.getName()));
////
////        }
//
//        Method m = null;
//        try {
//            m = demo.getMethod("setFrontBuffer", int.class, boolean.class);
//        } catch (NoSuchMethodException e) {
//            e.printStackTrace();
//        }
//
//        if (m != null)
//        {
//            try {
//                Log.i(TAG, "BasicGLSurfaceView: invoke method");
//                m.invoke(null, 123, true);
//            } catch (IllegalAccessException e) {
//                e.printStackTrace();
//            } catch (InvocationTargetException e) {
//                e.printStackTrace();
//            }
//        }

        //methods[0].invoke()
        //Log.i("BasicGLSurfaceView", "BasicGLSurfaceView: " + o.toString());

    }


    private static class Renderer implements GLSurfaceView.Renderer {

        GLSurfaceView view_ = null;

        public Renderer(GLSurfaceView view)
        {
            view_ = view;
        }

        public void onDrawFrame(GL10 gl)
        {
            //Native.step();
            gl.glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            gl.glClear(GL10.GL_COLOR_BUFFER_BIT);

        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            Surface sur = view_.getHolder().getSurface();
            Native.init(width, height, sur);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Do nothing.

        }
    }

}


class Native {
    static {
        // The runtime will add "lib" on the front and ".o" on the end of
        // the name supplied to loadLibrary.
        System.loadLibrary("simplejni");
    }

    static native int add(int a, int b);
    static native void step();
    static native void init(int width, int height, Object surface);
    static native Object nativeCreateSurface();

}

