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

import android.util.Log;

public class Patient {

    private String patientName, patientSurname, patientBirth, measurementId;
    private boolean saved;

    Patient() {
        patientName = "";
        patientSurname = "";
        patientBirth = "";
        measurementId = "";
        saved = false;
    }

    public void setPatientData(String name, String surname, String birth, String id) {
        patientName = name;
        patientSurname = surname;
        patientBirth = birth;
        measurementId = id;

        //String logi = String.format("New patient - id: %s, name: %s, surname: %s, birth: %s", measurementId, patientName, patientSurname, patientBirth);
        //Log.d("HEH", logi);
    }

    public void setSaved(boolean value) {
        saved = value;
    }

    public String getName() {
        return patientName;
    }

    public String getSurname() {
        return patientSurname;
    }

    public String getBirth() {
        return patientBirth;
    }

    public String getMeasurementId() {
        return measurementId;
    }

    public boolean getSaved() {
        return saved;
    }
}
