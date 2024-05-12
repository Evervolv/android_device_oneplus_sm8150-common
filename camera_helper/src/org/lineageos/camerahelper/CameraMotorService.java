/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.camerahelper;

import android.annotation.NonNull;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.camera2.CameraManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;

public class CameraMotorService extends Service implements Handler.Callback {
    private static final boolean DEBUG = true;
    private static final String TAG = "CameraMotorService";

    public static final int CAMERA_EVENT_DELAY_TIME = 100; // ms

    public static final String FRONT_CAMERA_ID = "1";

    public static final int MSG_CAMERA_CLOSED = 1000;
    public static final int MSG_CAMERA_OPEN = 1001;

    private Handler mHandler = new Handler(this);
    private CameraManager mCameraManager;
    private FallSensor mFallSensor;

    private long mClosedEvent;
    private long mOpenEvent;

    private CameraManager.AvailabilityCallback mAvailabilityCallback =
            new CameraManager.AvailabilityCallback() {
                @Override
                public void onCameraAvailable(@NonNull String cameraId) {
                    super.onCameraAvailable(cameraId);

                    if (cameraId.equals(FRONT_CAMERA_ID)) {
                        mClosedEvent = SystemClock.elapsedRealtime();
                        if (SystemClock.elapsedRealtime() - mOpenEvent < CAMERA_EVENT_DELAY_TIME
                                && mHandler.hasMessages(MSG_CAMERA_OPEN)) {
                            mHandler.removeMessages(MSG_CAMERA_OPEN);
                        }
                        mHandler.sendEmptyMessageDelayed(MSG_CAMERA_CLOSED,
                                CAMERA_EVENT_DELAY_TIME);
                    }
                }

                @Override
                public void onCameraUnavailable(@NonNull String cameraId) {
                    super.onCameraUnavailable(cameraId);

                    if (cameraId.equals(FRONT_CAMERA_ID)) {
                        mOpenEvent = SystemClock.elapsedRealtime();
                        if (SystemClock.elapsedRealtime() - mClosedEvent < CAMERA_EVENT_DELAY_TIME
                                && mHandler.hasMessages(MSG_CAMERA_CLOSED)) {
                            mHandler.removeMessages(MSG_CAMERA_CLOSED);
                        }
                        mHandler.sendEmptyMessageDelayed(MSG_CAMERA_OPEN,
                                CAMERA_EVENT_DELAY_TIME);
                    }
                }
            };

    private BroadcastReceiver mScreenStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_SCREEN_ON)) {
                if (DEBUG) Log.d(TAG, "Screen on, enabling fall sensor");
                mFallSensor.enable();
            } else if (action.equals(Intent.ACTION_SCREEN_OFF)) {
                if (DEBUG) Log.d(TAG, "Screen off, disabling fall sensor");
                mFallSensor.disable();
            }
        }
    };

    @Override
    public void onCreate() {
        CameraMotorController.calibrate();

        mCameraManager = getSystemService(CameraManager.class);
        mCameraManager.registerAvailabilityCallback(mAvailabilityCallback, null);

        mFallSensor = new FallSensor(this);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intent.ACTION_SCREEN_OFF);
        intentFilter.addAction(Intent.ACTION_SCREEN_ON);
        registerReceiver(mScreenStateReceiver, intentFilter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (DEBUG) Log.d(TAG, "Starting service");
        mFallSensor.enable();
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "Destroying service");
        mFallSensor.disable();
        unregisterReceiver(mScreenStateReceiver);
        mCameraManager.unregisterAvailabilityCallback(mAvailabilityCallback);
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public boolean handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_CAMERA_CLOSED:
            case MSG_CAMERA_OPEN:
                CameraMotorController.setOpen(msg.what == MSG_CAMERA_OPEN);
                break;
        }
        return true;
    }
}
