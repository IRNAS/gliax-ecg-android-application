/*
 * This file is part of MobilECG, an open source clinical grade Holter
 * ECG. For more information visit http://mobilecg.hu
 *
 * Copyright (C) 2016  Robert Csordas, Peter Isza
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.mobilecg.androidapp;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.SystemClock;
import android.util.DisplayMetrics;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;

import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.Cp21xxSerialDriver;
import com.hoho.android.usbserial.driver.ProbeTable;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.R.attr.id;

/**
 * Reading data from ECG board over USB
 * Original code made for bluetooth connection, modified for usb by vid553
 * Using modified version of usb-serial-for-android library by mik3y (https://github.com/mik3y/usb-serial-for-android)
 */

public class EcgActivity extends Activity {

    private boolean debugFileWrite = true; // set this to true to write signal data to files (do it also in EcgProcessor.cpp file)

    private GLSurfaceView mView;
    private DisplayMetrics displayMetrics;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private SerialInputOutputManager serialIoManager;
    private UsbManager usbManager = null;
    private UsbSerialDriver usbDriver = null;
    private UsbSerialPort serialPort = null;
    private ProbeTable customTable = null;
    private boolean render_paused = false;

    private static final String ACTION_USB_PERMISSION = "com.mobileecg.androidapp.USB_PERMISSION";
    private final String TAG = EcgActivity.class.getSimpleName();
    private IntentFilter intentFilter = null;
    private String debugFilePath = "";

    // initial values for usb communication parameters
    private int BAUD_RATE = 115200;
    private int DATA_BITS = 8;
    private int STOP_BITS = UsbSerialPort.STOPBITS_1;
    private int PARITY = UsbSerialPort.PARITY_NONE;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        if (debugFileWrite == true) {
            debugFilePath = this.getFilesDir().getAbsolutePath();
        }

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // Hide both the navigation bar and the status bar.
        // SYSTEM_UI_FLAG_FULLSCREEN is only available on Android 4.1 and higher, but as
        // a general rule, you should design your app to hide the status bar whenever you
        // hide the navigation bar.
        if(Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19) { // lower api
            View v = this.getWindow().getDecorView();
            v.setSystemUiVisibility(View.GONE);
        } else if(Build.VERSION.SDK_INT >= 19) {
            //for new api versions.
            View decorView = getWindow().getDecorView();
            int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
            decorView.setSystemUiVisibility(uiOptions);
        }

        displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

        mView = new GLSurfaceView(getApplication());
        mView.setEGLContextClientVersion(2);
        mView.setEGLConfigChooser(new MultisampleConfig());

        mView.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                openOptionsMenu();
                return true;
            }
        });

        mView.setRenderer(new GLSurfaceView.Renderer() {
            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                gl.glEnable(gl.GL_LINE_SMOOTH);
                gl.glHint(gl.GL_LINE_SMOOTH_HINT, gl.GL_NICEST);
                EcgJNI.surfaceCreated();
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                gl.glEnable(gl.GL_LINE_SMOOTH);
                gl.glHint(gl.GL_LINE_SMOOTH_HINT, gl.GL_NICEST);
                EcgJNI.setDotPerCM(displayMetrics.xdpi / 2.54f, displayMetrics.ydpi / 2.54f);
                EcgJNI.surfaceChanged(width, height);
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                EcgJNI.drawFrame();
            }
        });

        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                EcgJNI.init(getAssets(), MainScreen.GetSelectedMainsFreq());
                Log.d(TAG, "run event - onCreate");
                EcgJNI.initNDK(debugFilePath);
            }
        });

        setContentView(mView);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (usbReceiver != null) {
            unregisterReceiver(usbReceiver);
        }
        mView.onPause();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                EcgJNI.pause();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        intentFilter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(usbReceiver,intentFilter);
        customTable = CreateDevicesTable();
        FindUsbDevice();
        registerReceiver(usbReceiver,intentFilter);
        mView.onResume();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                EcgJNI.resume();
            }
        });
        Log.d(TAG, "run event - onResume");
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (debugFileWrite == true) {
            CopyDebugFiles();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main_menu, menu);//Menu Resource, Menu
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (render_paused) {
            menu.findItem(R.id.menu_btn1).setTitle(R.string.menu_button_1);
        }
        else {
            menu.findItem(R.id.menu_btn1).setTitle(R.string.menu_button_1_alt);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_btn1:    // Pause or resume measurement - freeze
                if (render_paused) {
                    mView.onResume();
                    mView.queueEvent(new Runnable() {
                        @Override
                        public void run() {
                            EcgJNI.resume();
                        }
                    });
                    render_paused = false;
                }
                else {
                    mView.onPause();
                    mView.queueEvent(new Runnable() {
                        @Override
                        public void run() {
                            EcgJNI.pause();
                        }
                    });
                    render_paused = true;
                }
                return true;
            case R.id.menu_btn2:    // Stop measurement - exit
                // TODO Save to pdf
                finish();
                System.exit(0);
                super.onStop();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    private ProbeTable CreateDevicesTable() {
        ProbeTable probeTable = new ProbeTable();
        probeTable.addProduct(0x0483, 0x374B, CdcAcmSerialDriver.class);   // add ST-LINK/V2.1 device support
        probeTable.addProduct(0x0483, 0x3748, CdcAcmSerialDriver.class);   // add ST-LINK/V2.0 device support
        probeTable.addProduct(0x0483, 0x3744, CdcAcmSerialDriver.class);   // add ST-LINK/V1.0 device support
        probeTable.addProduct(0x10C4, 0xEA60, Cp21xxSerialDriver.class);   // add UART bridge controller support
        return probeTable;
    }

    private void CopyDebugFiles() {
        if (isExternalStorageReadable()) {
            File outPath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS);
            File outFile = new File(outPath, "after_filters.txt");
            String inPath = debugFilePath + "/" + "after_filters.txt";
            File outFile2 = new File(outPath, "before_filters.txt");
            String inPath2 = debugFilePath + "/" + "before_filters.txt";
            try {
                // Copying file after filters
                FileInputStream inStream = new FileInputStream(inPath);
                byte[] buffer = new byte[inStream.available()];
                inStream.read(buffer);
                FileOutputStream outStream = new FileOutputStream(outFile);
                outStream.write(buffer);
                inStream.close();
                outStream.close();
                // Copying file before filters
                FileInputStream inStream2 = new FileInputStream(inPath2);
                byte[] buffer2 = new byte[inStream2.available()];
                inStream2.read(buffer2);
                FileOutputStream outStream2 = new FileOutputStream(outFile2);
                outStream2.write(buffer2);
                inStream2.close();
                outStream2.close();
            } catch (Exception e) {
                e.printStackTrace();
                Log.e(TAG, "ERROR: Copying debug files failed.");
            }
        }
        else {
            Log.e(TAG, "ERROR: External storage is not available.");
        }
    }

    public boolean isExternalStorageReadable() {
        String state = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(state) || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
            return true;
        }
        return false;
    }

    private void FindUsbDevice() {
        UsbSerialProber prober = new UsbSerialProber(customTable);
        usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> usbDriversList = prober.findAllDrivers(usbManager);
        if (usbDriversList.isEmpty()) {
            Log.d(TAG, "Usb device not connected.");
            return;
        }
        else {
            if (usbDriversList.size() > 1)  {
                Log.d(TAG, "More than one ECG devices connected. Selecting the first one...");
            }
            usbDriver = usbDriversList.get(0);
            PendingIntent pendingIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);
            usbManager.requestPermission(usbDriver.getDevice(), pendingIntent);
            Log.d(TAG, " Target device connected.");
        }
    }

    private void ConnectToUsbDevice() {
        UsbDeviceConnection deviceConnection = usbManager.openDevice(usbDriver.getDevice());
        if (deviceConnection == null) {
            Log.e(TAG, " Opening device connection failed.");
            return;
        }
        int numOfPorts = usbDriver.getPorts().size();
        if ( numOfPorts > 1) {
            Log.d(TAG, "Device has " + numOfPorts + " ports. Selecting the first one...");
        }
        serialPort = usbDriver.getPorts().get(0);
        if (serialPort != null) {
            try {
                serialPort.open(deviceConnection);
                serialPort.setParameters(BAUD_RATE, DATA_BITS, STOP_BITS, PARITY);

            } catch (IOException e) {
                Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
                try {
                    serialPort.close();
                } catch (IOException e2) {
                    // Ignore
                }
                serialPort = null;
                return;
            }
            onDeviceStateChange();
        }
        else {
            Log.e(TAG, " Opening device port failed.");
        }
    }

    private void onDeviceStateChange() {
        stopIoManager();
        startIoManager();
    }

    private void startIoManager() {
        if (serialPort != null) {
            Log.i(TAG, "Starting io manager ..");
            EcgJNI.onDeviceConnected();
            serialIoManager = new SerialInputOutputManager(serialPort, mListener);
            mExecutor.submit(serialIoManager);
        }
    }

    private void stopIoManager() {
        if (serialIoManager != null) {
            Log.i(TAG, "Stopping io manager ..");
            EcgJNI.onDeviceDisconnected();
            serialIoManager.stop();
            serialIoManager = null;
        }
    }

    private void updateReceivedData(byte[] data) {
        int length = data.length;
        //Log.i(TAG, "Read " + length + " bytes");
        try {
            EcgJNI.processEcgData(data, length);
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "ERROR: Reading data failed!");
        }
    }

    private final SerialInputOutputManager.Listener mListener = new SerialInputOutputManager.Listener() {
        @Override
        public void onNewData(final byte[] data) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    updateReceivedData(data);
                }
            });
        }

        @Override
        public void onRunError(Exception e) {
            Log.d(TAG, "Runner stopped.");
        }
    };

    private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                    ConnectToUsbDevice();
                }
                else {
                    Log.d(TAG, "Permission denied for accessing target device.");
                }
            }
        }

    };
}
