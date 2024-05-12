/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.camerahelper;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.KeyEvent;
import android.view.WindowManager;

import com.android.internal.os.DeviceKeyHandler;

public class KeyHandler implements DeviceKeyHandler {
    private static final String TAG = KeyHandler.class.getSimpleName();

    // Camera motor event key codes
    private static final int MOTOR_EVENT_MANUAL_TO_DOWN = 184;
    private static final int MOTOR_EVENT_UP_ABNORMAL = 186;
    private static final int MOTOR_EVENT_DOWN_ABNORMAL = 189;

    private final Context mContext;

    public KeyHandler(Context context) {
        mContext = context;
    }

    public KeyEvent handleKeyEvent(KeyEvent event) {
        int scanCode = event.getScanCode();

        switch (scanCode) {
            case MOTOR_EVENT_MANUAL_TO_DOWN:
            case MOTOR_EVENT_UP_ABNORMAL:
            case MOTOR_EVENT_DOWN_ABNORMAL:
                if (event.getAction() == KeyEvent.ACTION_DOWN) {
                    if (scanCode == MOTOR_EVENT_MANUAL_TO_DOWN) {
                        closeAllApps();
                    }
                    new Handler(Looper.getMainLooper()).post(() -> {
                        AlertDialog alertDialog = showCameraMotorWarning(scanCode);
                        if (alertDialog != null) {
                            alertDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                            alertDialog.setCanceledOnTouchOutside(false);
                            alertDialog.show();
                        }
                    });
                }
                return null;
            default:
                return event;
        }
    }

    private void closeAllApps() {
        // Go back to home to close all camera apps first
        Intent intent = new Intent(Intent.ACTION_MAIN);
        intent.addCategory(Intent.CATEGORY_HOME);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }

    private AlertDialog showCameraMotorWarning(int scanCode) {
        Context packageContext = null;
        try {
            packageContext = mContext.createPackageContext("org.lineageos.camerahelper", 0);
        } catch (NameNotFoundException | SecurityException e) {
            Log.e(TAG, "Failed to create package context", e);
        }

        if (packageContext != null)
            return null;

        AlertDialog.Builder builder = new AlertDialog.Builder(packageContext);
        switch (scanCode) {
            case MOTOR_EVENT_MANUAL_TO_DOWN:
                builder.setTitle(R.string.warning);
                builder.setMessage(R.string.motor_press_message);
                builder.setPositiveButton(android.R.string.ok, null);
                break;
            case MOTOR_EVENT_UP_ABNORMAL:
                builder.setTitle(R.string.warning);
                builder.setMessage(R.string.motor_cannot_go_up_message);
                builder.setNegativeButton(R.string.retry, (dialog, which) -> {
                    CameraMotorController.setOpen(true);
                });
                builder.setPositiveButton(R.string.close, (dialog, which) -> {
                    CameraMotorController.setOpen(false);
                    closeAllApps();
                });
                break;
            case MOTOR_EVENT_DOWN_ABNORMAL:
                builder.setTitle(R.string.warning);
                builder.setMessage(R.string.motor_cannot_go_down_message);
                builder.setPositiveButton(R.string.retry, (dialog, which) -> {
                    // Close the camera
                    CameraMotorController.setOpen(false);
                });
                break;
            default:
                return null;
        }
        return builder.create();
    }
}
