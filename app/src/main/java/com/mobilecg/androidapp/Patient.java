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

        String logi = String.format("New patient - id: %s, name: %s, surname: %s, birth: %s", measurementId, patientName, patientSurname, patientBirth);
        Log.d("HEH", logi);
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
