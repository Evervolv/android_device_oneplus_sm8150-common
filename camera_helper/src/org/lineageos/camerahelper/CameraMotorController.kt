/*
 * Copyright (c) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.camerahelper

import android.util.Log
import java.io.BufferedReader
import java.io.FileReader
import java.io.FileWriter
import java.io.IOException

object CameraMotorController {
    private const val TAG = "CameraMotorController"

    // Camera motor paths
    private const val CAMERA_MOTOR_ENABLE_PATH = "/sys/class/motor/enable"
    private const val CAMERA_MOTOR_HALL_CALIBRATION = "/sys/class/motor/hall_calibration"
    private const val CAMERA_MOTOR_DIRECTION_PATH = "/sys/class/motor/direction"
    private const val CAMERA_MOTOR_POSITION_PATH = "/sys/class/motor/position"

    // Motor calibration data path
    private const val CAMERA_PERSIST_HALL_CALIBRATION =
        "/mnt/vendor/persist/engineermode/hall_calibration"

    // Motor fallback calibration data
    private const val HALL_CALIBRATION_DEFAULT = "170,170,480,0,0,480,500,0,0,500,1500"

    private fun writeFile(path: String, value: String) {
        try {
            FileWriter(path).use { fileWriter ->
                fileWriter.write(value)
            }
        } catch (e: IOException) {
            Log.e(TAG, "Failed to write to $path", e)
        }
    }

    @JvmStatic
    fun calibrate() {
        var calibrationData = HALL_CALIBRATION_DEFAULT
        try {
            BufferedReader(FileReader(CAMERA_PERSIST_HALL_CALIBRATION)).use { reader ->
                calibrationData = reader.readLine()
            }
        } catch (e: IOException) {
            Log.e(TAG, "Failed to read from $CAMERA_PERSIST_HALL_CALIBRATION", e)
        }
        writeFile(CAMERA_MOTOR_HALL_CALIBRATION, calibrationData)
    }

    @JvmStatic
    fun getMotorPosition(): String? {
        try {
            BufferedReader(FileReader(CAMERA_MOTOR_POSITION_PATH)).use { reader ->
                return reader.readLine()
            }
        } catch (e: IOException) {
            Log.e(TAG, "Failed to read from $CAMERA_MOTOR_POSITION_PATH", e)
        }
        return null
    }

    @JvmStatic
    fun setMotorDirection(direction: String) {
        writeFile(CAMERA_MOTOR_DIRECTION_PATH, direction)
        writeFile(CAMERA_MOTOR_ENABLE_PATH, "1")
    }
}
