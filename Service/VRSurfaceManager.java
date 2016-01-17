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

    static {
        // The runtime will add "lib" on the front and ".o" on the end of
        // the name supplied to loadLibrary.
        System.loadLibrary("vr");
    }

	public VRSurfaceManager()
	{
		Log.i(TAG, "VRSurfaceManager: construct");
	}

	static public void setFrontBuffer(int surface, boolean bSet)
	{
		Log.i(TAG, String.format("setFrontBuffer: surface %d set %d", surface, bSet ? 1 : 0));
        nativeSetFrontBuffer(surface, bSet);
	}

	static private native void nativeSetFrontBuffer(int surface, boolean bSet);

//	setFrontBufferID = env->GetStaticMethodID( surfaceClass, "setFrontBuffer", "(IZ)V" );
//	getFrontBufferAddressID = env->GetStaticMethodID( surfaceClass, "getFrontBufferAddress", "(I)I" );
//	getSurfaceBufferAddressID = env->GetStaticMethodID( surfaceClass, "getSurfaceBufferAddress", "(I[II)I" );
//	getClientBufferAddressID = env->GetStaticMethodID( surfaceClass, "getClientBufferAddress", "(I)I" );



}
