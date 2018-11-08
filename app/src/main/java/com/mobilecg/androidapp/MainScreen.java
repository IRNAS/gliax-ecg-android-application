package com.mobilecg.androidapp;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.RelativeLayout;
import android.widget.Spinner;

import static android.R.attr.gravity;
import static android.R.attr.height;
import static android.R.attr.width;
import static android.content.ContentValues.TAG;

/**
 * Created by vidra on 6. 11. 2018.
 */

public class MainScreen extends Activity {
    // initial values for operational parameters
    private static int SELECTED_MAINS_FREQ = 0; // mains freq is 50 Hz
    private static int SELECTED_X_SPEED = 0;    // speed is 25 mm/s
    private static boolean PATIENT_DATA = true; // we request patient data

    @Override
    protected void onCreate (Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_screen);
    }

    public void MainBtnClick(View v) {
        if (v.getId() == R.id.StartBtn) {
            final Intent i = new Intent(this, EcgActivity.class);
            if (PATIENT_DATA) {
                LayoutInflater layoutInflater = (LayoutInflater) getApplicationContext().getSystemService(LAYOUT_INFLATER_SERVICE);
                ViewGroup container = (ViewGroup) layoutInflater.inflate(R.layout.input_data_popup, null);

                DisplayMetrics displayMetrics = new DisplayMetrics();
                getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
                int width = displayMetrics.widthPixels;
                int height = displayMetrics.heightPixels;

                final PopupWindow popupWindow = new PopupWindow(container,(int)(width * .8), (int)(height * .6));
                LinearLayout linearLayout = (LinearLayout) findViewById(R.id.layout_main);
                popupWindow.showAtLocation(linearLayout, Gravity.NO_GRAVITY, width/10, height/5);

                linearLayout.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View view, MotionEvent motionEvent) {
                        popupWindow.dismiss();
                        return true;
                    }
                });

                Button ok_btn = (Button) findViewById(R.id.buttonPopUp);    // TODO fix it
                if (ok_btn != null) {
                    ok_btn.setOnClickListener(new View.OnClickListener() {
                        @Override
                        public void onClick(View view) {
                            startActivity(i);
                        }
                    });
                }
            }
            else {
                startActivity(i);
            }
        }
        else if (v.getId() == R.id.SettingsBtn) {
            EditSettings();
        }
        else if (v.getId() == R.id.OldDataBtn) {
            Log.i(TAG, "Old data request");
        }
        else if (v.getId() == R.id.ExitBtn) {
            finish();
            System.exit(0);
        }
    }

    private void EditSettings() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        View view = getLayoutInflater().inflate(R.layout.settings_popup, null);

        final Spinner mainsFreqSpinner = (Spinner) view.findViewById(R.id.spinnerMains);
        final ArrayAdapter<CharSequence> mainsFreqAdapter = ArrayAdapter.createFromResource(view.getContext(), R.array.mains, android.R.layout.simple_spinner_item);
        mainsFreqAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mainsFreqSpinner.setAdapter(mainsFreqAdapter);

        final Spinner xSpeedSpinner = (Spinner) view.findViewById(R.id.spinnerSpeedX);
        final ArrayAdapter<CharSequence> xSpeedAdapter = ArrayAdapter.createFromResource(view.getContext(), R.array.x_speed, android.R.layout.simple_spinner_item);
        xSpeedAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        xSpeedSpinner.setAdapter(xSpeedAdapter);

        final CheckBox patientDataBox = (CheckBox) view.findViewById(R.id.patientDataBox);
        patientDataBox.setChecked(PATIENT_DATA);

        mainsFreqSpinner.setSelection(SELECTED_MAINS_FREQ);
        xSpeedSpinner.setSelection(SELECTED_X_SPEED);

        builder.setView(view);
        final AlertDialog alertDialog = builder.create();

        Button btnOK = (Button) view.findViewById(R.id.btnOK);
        btnOK.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (SELECTED_MAINS_FREQ != mainsFreqSpinner.getSelectedItemPosition()) {
                    SELECTED_MAINS_FREQ = mainsFreqSpinner.getSelectedItemPosition();
                    Log.i(TAG, "Mains: " + GetSelectedMainsFreq());
                }
                if (SELECTED_X_SPEED != xSpeedSpinner.getSelectedItemPosition()) {
                    SELECTED_X_SPEED = xSpeedSpinner.getSelectedItemPosition();
                    Log.i(TAG, "X speed: " + GetSelectedXspeed());
                }
                PATIENT_DATA = patientDataBox.isChecked();
                Log.i(TAG, "Patient data: " + PATIENT_DATA);
                alertDialog.dismiss();
            }
        });
        alertDialog.show();
    }

    public static int GetSelectedMainsFreq() {
        int selected;
        switch(SELECTED_MAINS_FREQ) {   // in Hz
            case 0: selected = 50; break;
            case 1: selected = 60; break;
            default: selected = -1;
        }
        return selected;
    }

    public static int GetSelectedXspeed() {
        int selected;
        switch(SELECTED_X_SPEED) {  // in mm/s
            case 0: selected = 25; break;
            case 1: selected = 50; break;
            default: selected = -1;
        }
        return selected;
    }
}
