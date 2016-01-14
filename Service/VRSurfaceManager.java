package android.app;

import android.util.Log;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.StringTokenizer;

public class VRSurfaceManager {
    private static final String TAG = "VRSurfaceManager";

fr
	public VRSurfaceManager()
	{
		Log.i(TAG, "VRSurfaceManager: construct");
	}


	static public int foo(int a)
	{
		return a;
	}

	static public void setFrontBuffer(int surface, boolean bSet)
	{
		Log.i(TAG, String.format("setFrontBuffer: surface %d", surface));
        nativeSetFrontBuffer(surface);
	}


	static {
		// The runtime will add "lib" on the front and ".o" on the end of
		// the name supplied to loadLibrary.
		System.loadLibrary("vr");
	}

	static private native void nativeSetFrontBuffer(int surface);


//	setFrontBufferID = env->GetStaticMethodID( surfaceClass, "setFrontBuffer", "(IZ)V" );
//	getFrontBufferAddressID = env->GetStaticMethodID( surfaceClass, "getFrontBufferAddress", "(I)I" );
//	getSurfaceBufferAddressID = env->GetStaticMethodID( surfaceClass, "getSurfaceBufferAddress", "(I[II)I" );
//	getClientBufferAddressID = env->GetStaticMethodID( surfaceClass, "getClientBufferAddress", "(I)I" );



}
