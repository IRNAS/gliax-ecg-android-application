package com.mobilecg.androidapp;

import android.app.Application;
import android.content.Context;

public class EcgApplication extends Application {
    public static EcgApplication instance;

    @Override
    public void onCreate() {
        super.onCreate();
        instance = this;
    }

    @Override
    public Context getApplicationContext() {
        return super.getApplicationContext();
    }

    public static EcgApplication getInstance() {
        return instance;
    }
}
