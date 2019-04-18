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

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.net.Uri;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.SystemClock;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.DisplayMetrics;

import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;
import android.support.v4.content.ContextCompat;

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
import java.util.Arrays;
import java.util.Calendar;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.R.attr.id;
import static android.content.ContentValues.TAG;
import static com.mobilecg.androidapp.PopUps.GetSelectedXspeed;

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

    Button pause_resume_btn;
    Button save_btn;
    Button rhythm_12lead_btn;
    Button patient_btn;
    private boolean render_paused = false;
    private boolean rhythm_screen = false;

    private static final String ACTION_USB_PERMISSION = "com.mobileecg.androidapp.USB_PERMISSION";
    //private final String TAG = EcgActivity.class.getSimpleName();
    private final String TAG = "HEH";
    private IntentFilter intentFilter = null;
    private String debugFilePath = "";
    private final int APP_ALLOW_STORAGE = 1;

    // initial values for usb communication parameters
    private int BAUD_RATE = 115200;
    private int DATA_BITS = 8;
    private int STOP_BITS = UsbSerialPort.STOPBITS_1;
    private int PARITY = UsbSerialPort.PARITY_NONE;

    // patient data values
    private String patientName, patientSurname, patientBirth;
    private String measurementId, timestamp;
    // advanced settings values
    private static int paperSpeed = 25;    // speed is 25 mm/s - TODO display alert if not default speed
    private String saveLocation = "MobilECG";    // TODO make save location stay as it's set between app launches
    private boolean autoPrint = true;   // this gets reset between app launches

    @SuppressLint("ClickableViewAccessibility")
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        if (debugFileWrite == true) {
            debugFilePath = this.getFilesDir().getAbsolutePath();
        }

        displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

        setContentView(R.layout.ecg_layout);

        //mView = new GLSurfaceView(getApplication());
        mView = (GLSurfaceView) findViewById(R.id.gl_surface_view);

        mView.setEGLContextClientVersion(2);
        mView.setEGLConfigChooser(new MultisampleConfig());

        /*
        mView.setOnLongClickListener(new View.OnLongClickListener() {   // TODO
            @Override
            public boolean onLongClick(View v) {
                openOptionsMenu();
                return true;
            }
        });
        */
        /*
        mView.setOnClickListener(new View.OnClickListener() {   // TODO
            @Override
            public void onClick(View v) {
                float x = v.getX();
                float y = v.getY();
                String test = "X: " + x + " Y: " + y;
                //Log.d(TAG, test);
            }
        });
        */

        mView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                float x = event.getX();
                float y = event.getY();
                String test = "X: " + x + " Y: " + y;
                Log.d(TAG, "screen height: " + v.getHeight());
                Log.d(TAG, test);
                //EcgJNI.getButtonCoords();
                //EcgJNI.getButtonCoords();
                //Log.d(TAG, "button coords: " + test2[0]);
                return false;
            }
        });

        mView.setRenderer(new MyGLRenderer(displayMetrics));

        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                //EcgJNI.init(getAssets(), PopUps.GetSelectedMainsFreq());
                EcgJNI.init(getAssets(), 50);
                Log.d(TAG, "run event - onCreate");
                EcgJNI.initNDK(debugFilePath);
            }
        });

        // Main GUI buttons
        pause_resume_btn = (Button)findViewById(R.id.pause_btn);   // Pause / resume
        pause_resume_btn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (render_paused) {
                    resumeECG();
                }
                else {
                    pauseECG();
                }
            }
        });

        save_btn = (Button)findViewById(R.id.save_btn);    // Save
        save_btn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (!render_paused) {   // pause ecg
                    pauseECG();
                }
                if (!autoPrint) {   // display alert
                    showPrintAlertDialog();
                }
                else {
                    // TODO call print function
                }
            }
        });
        rhythm_12lead_btn = (Button)findViewById(R.id.rhythm_btn);    // Rhythm / 12 lead
        rhythm_12lead_btn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (!rhythm_screen) {   // 12 lead -> rhythm
                    // TODO call native cpp function
                    rhythm_12lead_btn.setText(R.string.menu_button_3_alt);
                    rhythm_screen = true;
                }
                else {  // rhythm -> 12 lead
                    // TODO call native cpp function
                    rhythm_12lead_btn.setText(R.string.menu_button_3);
                    rhythm_screen = false;
                }
            }
        });
        patient_btn = (Button)findViewById(R.id.patient_btn);  // Patient
        patient_btn.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                inputPatientData();
            }
        });

        // check if app has permission for storage read/write and request it if not
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, new String[] {Manifest.permission.WRITE_EXTERNAL_STORAGE}, APP_ALLOW_STORAGE);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch(requestCode) {
            case APP_ALLOW_STORAGE:
                if (grantResults.length == 0 || grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(this, "Permission denied for accessing external storage! You won't be able to save pdfs!", Toast.LENGTH_LONG).show();
                }
                break;
        }
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "run event - onPause");
       super.onPause();
       pauseECG();
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "run event - onResume");
        super.onResume();
        hideNavAndStatusBar();
        resumeECG();
    }

    @Override
    protected void onStop() {
        Log.d(TAG, "run event - onStop");
        super.onStop();
        if (debugFileWrite == true) {
            CopyDebugFiles();
        }
    }

    public void hideNavAndStatusBar() {
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
    }

    private void resumeECG() {
        Log.d(TAG, "resumeECG function");
        // resume reading from usb
        intentFilter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(usbReceiver,intentFilter);
        customTable = CreateDevicesTable();
        FindUsbDevice();

        // resume drawing
        mView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY); // resume auto rendering
        mView.onResume();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                EcgJNI.resume();
            }
        });
        if (render_paused) {
            render_paused = false;
            pause_resume_btn.setText(R.string.menu_button_1);
        }
    }

    private void pauseECG() {
        Log.d(TAG, "pauseECG - function");
        // TODO pause reading from usb ???
        unregisterReceiver(usbReceiver);
        //}

        // pause drawing
        mView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);   // this stops auto rendering
        mView.onPause();
        mView.queueEvent(new Runnable() {
            @Override
            public void run() {
                EcgJNI.pause();
            }
        });

        if (!render_paused) {
            pause_resume_btn.setText(R.string.menu_button_1_alt);
            render_paused = true;
        }
    }
    /*
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
    */

    private void showPrintAlertDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.question_print_measur);
        builder.setPositiveButton(R.string.print_btn, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // TODO call print function
            }
        });
        builder.setNegativeButton(R.string.cancel_btn, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                // return to main screen
                hideNavAndStatusBar();
            }
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private void inputPatientData() {   // TODO move to PopUps.java
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        View view = getLayoutInflater().inflate(R.layout.input_data_popup, null);

        builder.setView(view);
        final AlertDialog alertDialog = builder.create();

        final EditText etName = (EditText) view.findViewById(R.id.popup_name);
        final EditText etSurname = (EditText) view.findViewById(R.id.popup_surname);
        final EditText etBirth = (EditText) view.findViewById(R.id.birth_date);
        final EditText etMeasurementID = (EditText) view.findViewById(R.id.measurement_id);
        etName.setText(patientName);
        etSurname.setText(patientSurname);
        etBirth.setText(patientBirth);
        etMeasurementID.setText(measurementId);

        Button btnOK = (Button) view.findViewById(R.id.buttonPopUpOK);
        btnOK.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                patientName = etName.getText().toString().trim();
                patientSurname = etSurname.getText().toString().trim();
                patientBirth = etBirth.getText().toString().trim();
                measurementId  = etMeasurementID.getText().toString().trim();
                timestamp = java.text.DateFormat.getDateTimeInstance().format(Calendar.getInstance().getTime());
                alertDialog.dismiss();
                hideNavAndStatusBar();
                String logi = String.format("name: %s, surname: %s, birth: %s", patientName, patientSurname, patientBirth);
                Log.d(TAG, logi);
            }
        });

        Button btnAdvSet = (Button) view.findViewById(R.id.buttonPopUpAdvanced);    // TODO move to xml all button onclick methods
        btnAdvSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                advancedSettings();
            }
        });

        Button btnArchive = (Button) view.findViewById(R.id.buttonPopUpArchive);
        btnArchive.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                showArchive();
            }
        });

        Button btnNew = (Button) view.findViewById(R.id.buttonPopUpNew);
        btnNew.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                etName.setText("");
                etSurname.setText("");
                etBirth.setText("");
                etMeasurementID.setText("");
            }
        });

        alertDialog.show();
    }

    private void advancedSettings() {   // TODO move to PopUps.java
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        View view = getLayoutInflater().inflate(R.layout.advanced_popup, null);

        builder.setView(view);
        final AlertDialog alertDialog = builder.create();

        final EditText etSaveLoc = (EditText) view.findViewById(R.id.save_location);
        etSaveLoc.setText(saveLocation);
        final RadioButton rbSpeed25 = (RadioButton) view.findViewById(R.id.pap_speed_25);
        final RadioButton rbSpeed50 = (RadioButton) view.findViewById(R.id.pap_speed_50);
        if (paperSpeed == 25) {
            rbSpeed25.setChecked(true);
        }
        else {
            rbSpeed50.setChecked(true);
        }
        final ToggleButton tbAutoSave = (ToggleButton) view.findViewById(R.id.toggle1);
        tbAutoSave.setChecked(autoPrint);

        Button btnOK = (Button) view.findViewById(R.id.btnOK);
        btnOK.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String save_loc = etSaveLoc.getText().toString().trim();
                if (save_loc != saveLocation) { saveLocation = save_loc; }
                Log.i(TAG, "save location: " + save_loc);

                int paper_speed = 0;
                if (rbSpeed25.isChecked()) { paper_speed = 25; }
                else if (rbSpeed50.isChecked()) { paper_speed = 50; }
                if (paper_speed != paperSpeed) { paperSpeed = paper_speed; }
                Log.i(TAG, "Paper speed: " + paper_speed);

                if (tbAutoSave.isChecked()) { autoPrint = true; }
                else { autoPrint = false; }
                Log.i(TAG, "Auto print on save: " + autoPrint);

                alertDialog.dismiss();
            }
        });

        Button btnAbout = (Button) view.findViewById(R.id.btnAbout);
        btnAbout.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://www.irnas.eu"));
                startActivity(browserIntent);
            }
        });

        alertDialog.show();
    }

    private void showArchive() {
        if (isExternalStorageReadable()) {
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            View view = getLayoutInflater().inflate(R.layout.archive_popup, null);

            builder.setView(view);
            final AlertDialog alertDialog = builder.create();
            File dir = new File(Environment.getExternalStorageDirectory() + File.separator + saveLocation);
            if (!dir.exists()) {
                boolean result = dir.mkdirs();
                if (!result) {
                    Log.i(TAG, "archive mkdir error");
                    return;
                }
            }
            TextView textView = (TextView) view.findViewById(R.id.text_view_empty_archive);
            ListView listView = (ListView) view.findViewById(R.id.archive_list_view);
            File[] fileList = dir.listFiles();
            if (fileList != null && fileList.length > 0) {
                String[] namesOfFiles = new String[fileList.length];
                for (int i = 0; i < namesOfFiles.length; i++) {
                    namesOfFiles[i] = fileList[i].getName();
                    Log.i(TAG, "name: " + namesOfFiles[i]);
                }
                Arrays.sort(namesOfFiles);
                final ArrayAdapter<String> listOfFiles = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, namesOfFiles);
                listView.setAdapter(listOfFiles);
                textView.setVisibility(View.GONE);
            }
            else {
                listView.setVisibility(View.GONE);
                TextView infoTextView = new TextView(this);
                infoTextView.setLayoutParams(new WindowManager.LayoutParams(WindowManager.LayoutParams.WRAP_CONTENT, WindowManager.LayoutParams.WRAP_CONTENT));
                infoTextView.setText(getString(R.string.empty_archive));
            }
            alertDialog.show();
        }
        else {
            Toast.makeText(this, "Error accessing device storage...", Toast.LENGTH_SHORT).show();
        }
    }

    // TODO handle search function in archive

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

    private void disableMainButtons() {
        pause_resume_btn.setEnabled(false);
        save_btn.setEnabled(false);
        rhythm_12lead_btn.setEnabled(false);
    }

    private void enableMainButtons() {
        pause_resume_btn.setEnabled(true);
        save_btn.setEnabled(true);
        rhythm_12lead_btn.setEnabled(true);
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
            disableMainButtons();
            return;
        }
        else {
            enableMainButtons();
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
        public void onReceive(Context context, Intent intent) { // TODO handle usb detached and attached events
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                    ConnectToUsbDevice();
                }
                else {
                    Log.d(TAG, "Permission denied for accessing ECG device!");
                }
            }
        }
    };

    // TODO handle back button press (stop at main screen, hide nav bar)
}
