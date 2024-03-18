/*
 * Copyright (c) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.camerahelper

import android.app.AlertDialog
import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.WindowManager
import java.util.concurrent.Executors

class FallSensor(private val context: Context) : SensorEventListener {
    private val executorService = Executors.newSingleThreadExecutor()
    private val sensorManager = context.getSystemService(SensorManager::class.java)!!
    private var sensor = sensorManager.getSensorList(
        Sensor.TYPE_ALL
    ).find { it.stringType == "camera_protect" }

    override fun onSensorChanged(event: SensorEvent) {
        if (event.values[0] <= 0) return

        Log.d(TAG, "Fall detected, ensuring front camera is closed")

        // We shouldn't really bother doing anything if motor is already closed
        if (CameraMotorController.getMotorPosition() == "1") return

        // Close the camera
        CameraMotorController.setMotorDirection("0")

        // Show alert dialog informing user that we closed the camera
        executorService.submit {
            val dialog = AlertDialog.Builder(
                context
            ).apply {
                setTitle(R.string.free_fall_detected_title)
                setMessage(R.string.free_fall_detected_message)
                setNegativeButton(
                    R.string.raise_the_camera
                ) { _: DialogInterface?, _: Int ->
                    // Reopen the camera
                    CameraMotorController.setMotorDirection("1")
                }
                .setPositiveButton(
                    R.string.close
                ) { _: DialogInterface?, _: Int ->
                    // Go back to home screen
                    context.startActivity(Intent(Intent.ACTION_MAIN).apply {
                        addCategory(Intent.CATEGORY_HOME)
                        setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
                    })
                }
            }.create()

            dialog.apply {
                window?.apply {
                    setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY)
                    setCanceledOnTouchOutside(false)
                    show()
                }
            }
        }
    }

    override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) { }

    fun enable() {
        if (DEBUG) Log.d(TAG, "Enabling")
        executorService.submit {
            sensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_FASTEST)
        }
    }

    fun disable() {
        if (DEBUG) Log.d(TAG, "Disabling")
        executorService.submit {
            sensorManager.unregisterListener(this, sensor)
        }
    }

    companion object {
        private const val DEBUG = true
        private const val TAG = "FallSensor"
    }
}
