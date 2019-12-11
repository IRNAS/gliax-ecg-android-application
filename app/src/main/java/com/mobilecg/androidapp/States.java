/*
 * This file is part of gliax-ecg-android-application
 * Glia is a project with the goal of releasing high quality free/open medical hardware
 * to increase availability to those who need it.
 * For more information visit Glia Free Medical hardware webpage: https://glia.org/
 *
 * Made by Institute Irnas (https://www.irnas.eu/)
 * Copyright (C) 2019 Vid Rajtmajer
 *
 * Based on MobilECG, an open source clinical grade Holter ECG.
 * For more information visit http://mobilecg.hu
 * Authors: Robert Csordas, Peter Isza
 *
 * This project uses modified version of usb-serial-for-android driver library
 * to communicate with Irnas made ECG board.
 * Original source code: https://github.com/mik3y/usb-serial-for-android
 * Library made by mik3y and kai-morich, modified by Vid Rajtmajer
 * Licensed under LGPL Version 2.1
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
}
