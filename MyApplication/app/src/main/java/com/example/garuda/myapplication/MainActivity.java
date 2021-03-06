package com.example.garuda.myapplication;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.EditText;

import java.io.IOException;
import java.util.Set;
import java.util.UUID;




public class MainActivity extends Activity implements SurfaceHolder.Callback {

    static {
        // The runtime will add "lib" on the front and ".o" on the end of
        // the name supplied to loadLibrary.
        System.loadLibrary("simplejni");
    }

    private static final int REQUEST_ENABLE_BT = 1;
    private static final UUID MY_UUID = UUID.randomUUID();
    private static final String TAG = "My Application";
    protected String text_ = "";
    ConnectThread connThread_;

    protected void AddText(String text) {
        Log.i(TAG, text);
        text_ += text;
        text_ += "\r\n";
        EditText edit = (EditText)findViewById(R.id.editText);
        edit.setText(text_);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //SurfaceView sv = new SurfaceView( this );
        //setContentView( sv );
        //sv.getHolder().addCallback(this);

        Window window = getWindow();
        WindowManager.LayoutParams params = window.getAttributes();
        params.systemUiVisibility = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
        window.setAttributes(params);

        setContentView(new BasicGLSurfaceView(this));

/*
        setContentView(R.layout.activity_main);

        AddText("start");
        //int sum = Native.add(2, 3);
        //AddText("2 + 3 = " + Integer.toString(sum));

        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null) {
            AddText("bluetooth adapter is null");
            return;
        }

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();

        BluetoothDevice myDevice = null;

// If there are paired devices
        if (pairedDevices.size() > 0) {
            // Loop through paired devices
            for (BluetoothDevice device : pairedDevices) {
                // Add the name and address to an array adapter to show in a ListView
                //mArrayAdapter.add();

                AddText(device.getName() + " " + device.getAddress() + " " + device.getUuids()[0]);
                if (device.getName().equals("AM-08")) {
                    myDevice = device;
                }
            }
        }

        if (myDevice != null) {
            AddText("connect to device...");
            connThread_ = new ConnectThread(mBluetoothAdapter, myDevice);
            connThread_.start();
        }*/
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Native.init(width, height, holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.e(TAG, "start onStart~~~");
        //Native.nativeStart();
    }
    @Override
    protected void onRestart() {
        super.onRestart();
        Log.e(TAG, "start onRestart~~~");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e(TAG, "start onResume~~~");
    }
    @Override
    protected void onPause() {
        super.onPause();
        Log.e(TAG, "start onPause~~~");
    }
    @Override
    protected void onStop() {
        super.onStop();
        Log.e(TAG, "start onStop~~~");
        //Native.nativeStop();
    }


    private class ConnectThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;

        private BluetoothAdapter mBluetoothAdapter;

        public ConnectThread(BluetoothAdapter adapter, BluetoothDevice device) {
            mBluetoothAdapter = adapter;
            // Use a temporary object that is later assigned to mmSocket,
            // because mmSocket is final
            BluetoothSocket tmp = null;
            mmDevice = device;

            // Get a BluetoothSocket to connect with the given BluetoothDevice
            try {
                // MY_UUID is the app's UUID string, also used by the server code
                tmp = device.createRfcommSocketToServiceRecord(MY_UUID);
            } catch (IOException e) {
                AddText("createRfcommSocketToServiceRecord error");
            }

            mmSocket = tmp;
            AddText("mmSocket " + mmSocket);
        }

        public void run() {
            AddText("run...");
            // Cancel discovery because it will slow down the connection
            //mBluetoothAdapter.cancelDiscovery();

            try {
                // Connect the device through the socket. This will block
                // until it succeeds or throws an exception
                mmSocket.connect();
            } catch (IOException connectException) {
                // Unable to connect; close the socket and get out
                try {
                    mmSocket.close();
                } catch (IOException closeException) {
                }
                AddText("connect failed" + connectException.toString());

                return;
            }

            AddText("connected !");

            // Do work to manage the connection (in a separate thread)
            //manageConnectedSocket(mmSocket);
        }

        /** Will cancel an in-progress connection, and close the socket */
        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) { }
        }
    }



    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);

        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.d
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }


        return super.onOptionsItemSelected(item);
    }
}

