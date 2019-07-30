package com.mobilecg.androidapp;

import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Build;
import android.util.Log;
import android.widget.Toast;

public class BatteryDetectReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(final Context context, Intent intent) { // TODO call getAction
        if (isCharging(context)) {
            Toast.makeText(context, "Tablet is being charged...", Toast.LENGTH_SHORT).show();
        }
        else {
            // TODO display this alert activity only if ECG is actually connected
            Intent intent1 = new Intent(context, AlertActivity.class);
            intent1.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intent1);
        }
    }

    public static boolean isCharging(Context context) {
        IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = context.registerReceiver(null, ifilter);
        int status = batteryStatus.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        boolean bCharging = status == BatteryManager.BATTERY_STATUS_CHARGING || status == BatteryManager.BATTERY_STATUS_FULL;
        return bCharging;
    }

}
