/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.camerahelper

import android.app.Service
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.hardware.camera2.CameraManager
import android.hardware.camera2.CameraManager.AvailabilityCallback
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.Message
import android.os.SystemClock
import android.util.Log

class CameraMotorService : Service(), Handler.Callback {
    private lateinit var cameraManager: CameraManager
    private lateinit var fallSensor: FallSensor

    private val handler = Handler(Looper.myLooper()!!, this)
    
    private val availabilityCallback: AvailabilityCallback = object : AvailabilityCallback() {
        private var closedEvent: Long = 0
        private var openEvent: Long = 0
        
        private fun handleCameraEvent(delta: Long, available: Boolean) {
            val message = if (available) MSG_CAMERA_OPEN else MSG_CAMERA_CLOSED
            if (delta < MSG_CAMERA_DELAY_MS
                && handler.hasMessages(message)
            ) {
                handler.removeMessages(message)
            }
            handler.sendEmptyMessageDelayed(
                if (available) MSG_CAMERA_CLOSED else MSG_CAMERA_OPEN,
                MSG_CAMERA_DELAY_MS
            )
        }

        override fun onCameraAvailable(cameraId: String) {
            super.onCameraAvailable(cameraId)
            if (cameraId != "1") return
            closedEvent = SystemClock.elapsedRealtime()
            handleCameraEvent(closedEvent - openEvent, true)
        }

        override fun onCameraUnavailable(cameraId: String) {
            super.onCameraUnavailable(cameraId)
            if (cameraId != "1") return
            openEvent = SystemClock.elapsedRealtime()
            handleCameraEvent(openEvent - closedEvent, false)
        }
    }

    private val screenStateReceiver: BroadcastReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            when (intent.action) {
                Intent.ACTION_SCREEN_ON -> fallSensor.enable()
                Intent.ACTION_SCREEN_OFF -> fallSensor.disable()
            }
        }
    }

    override fun onCreate() {
        Log.d(TAG, "Creating service")
        CameraMotorController.calibrate()
        cameraManager = getSystemService(CameraManager::class.java)!!
        cameraManager.registerAvailabilityCallback(availabilityCallback, null)

        fallSensor = FallSensor(this)
        val screenStateFilter = IntentFilter()
        screenStateFilter.addAction(Intent.ACTION_SCREEN_OFF)
        screenStateFilter.addAction(Intent.ACTION_SCREEN_ON)
        registerReceiver(screenStateReceiver, screenStateFilter)
    }

    override fun onStartCommand(intent: Intent, flags: Int, startId: Int): Int {
        fallSensor.enable()
        return START_STICKY
    }

    override fun onDestroy() {
        super.onDestroy()
        fallSensor.disable()
        unregisterReceiver(screenStateReceiver)
        cameraManager.unregisterAvailabilityCallback(availabilityCallback)
    }

    override fun onBind(intent: Intent): IBinder? { return null }

    override fun handleMessage(msg: Message): Boolean {
        when (msg.what) {
            MSG_CAMERA_CLOSED ->
                CameraMotorController.setMotorDirection("0")
            MSG_CAMERA_OPEN ->
                CameraMotorController.setMotorDirection("1")
        }
        return true
    }

    companion object {
        private const val TAG = "CameraMotorService"
        private const val MSG_CAMERA_DELAY_MS = 100L
        private const val MSG_CAMERA_CLOSED = 1000
        private const val MSG_CAMERA_OPEN = 1001
    }
}
