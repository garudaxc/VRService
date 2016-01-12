/**
 * Created by garuda on 12/14/15.
 */

package com.example.garuda.myapplication;

import android.content.Context;
import android.opengl.GLSurfaceView;

import android.opengl.GLES20;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class BasicGLSurfaceView extends GLSurfaceView {
    public BasicGLSurfaceView(Context context) {
        super(context);
        //setEGLContextClientVersion(2);
        setRenderer(new Renderer());
    }



    private static class Renderer implements GLSurfaceView.Renderer {

        public void onDrawFrame(GL10 gl)
        {
            //Native.step();
            gl.glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
            gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
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

