/**
 * Created by garuda on 12/14/15.
 */

package com.example.garuda.myapplication;

import android.content.Context;
import android.opengl.GLSurfaceView;

import android.opengl.GLES20;
import android.util.Log;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class BasicGLSurfaceView extends GLSurfaceView {
    static String TAG = "BasicGLSurfaceView";

    public BasicGLSurfaceView(Context context) {
        super(context);
        //setEGLContextClientVersion(2);
        setRenderer(new Renderer());

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

        public void onDrawFrame(GL10 gl)
        {
            //Native.step();
            //gl.glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            //gl.glClear(GL10.GL_COLOR_BUFFER_BIT);

        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {

            Native.init(width, height);
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
    static native void init(int width, int height);

}

