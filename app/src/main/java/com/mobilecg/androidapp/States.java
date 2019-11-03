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

public class States {
    // initial values for operational parameters
    private static boolean ECG_RUNNING = false;
    private static boolean ECG_CONNECTED = false;

    private static int PAPER_SPEED = 25;
    //private static int SELECTED_MAINS_FREQ = 0; // mains freq is 50 Hz

    public static int SHOT_PREPARE = 0;
    public static int SHOT_ONE = 1;
    public static int SHOT_MANY = 2;

    // disconnect broadcast receiver
    public static final String INTENT_ACTION_DISCONNECT = BuildConfig.APPLICATION_ID + ".Disconnect";

    // Getters
    public static boolean isEcgRunning() {
        return ECG_RUNNING;
    }
    public static boolean isEcgConnected() {
        return ECG_CONNECTED;
    }

    public static int getPaperSpeed() { return PAPER_SPEED; }

    // Setters
    public static void setEcgRunning(boolean state) {
        ECG_RUNNING = state;
    }
    public static void setEcgConnected(boolean ecgConnected) { ECG_CONNECTED = ecgConnected; }

    public static void setPaperSpeed(int speed) {
        PAPER_SPEED = speed;
        if (speed == 50) {
            EcgJNI.ChangeSpeed(1.25f);
        }
        else {
            EcgJNI.ChangeSpeed((float) speed/10);
        }
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
