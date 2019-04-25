package com.mobilecg.androidapp;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;

import java.util.Calendar;

import static android.content.ContentValues.TAG;

/**
 * Created by vidra on 6. 11. 2018.
 */

public class PopUps extends Activity {
    // initial values for operational parameters
    //private static int SELECTED_MAINS_FREQ = 0; // mains freq is 50 Hz
    private static int SELECTED_X_SPEED = 0;    // speed is 25 mm/s

    private void inputPatientData() {

    }

    public void advancedSettings() {

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

    /*
    public static int GetSelectedMainsFreq() {
        int selected;
        switch(SELECTED_MAINS_FREQ) {   // in Hz
            case 0: selected = 50; break;
            case 1: selected = 60; break;
            default: selected = -1;
        }
        return selected;
    }
    */
}
