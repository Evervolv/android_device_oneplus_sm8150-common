/*
 * Copyright (c) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.camerahelper;

import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class CameraMotorController {
    private static final String TAG = "CameraMotorController";

    // Camera motor paths
    private static final String CAMERA_MOTOR_ENABLE_PATH =
            "/sys/class/motor/enable";
    private static final String CAMERA_MOTOR_HALL_CALIBRATION =
            "/sys/class/motor/hall_calibration";
    private static final String CAMERA_MOTOR_DIRECTION_PATH =
            "/sys/class/motor/direction";
    private static final String CAMERA_MOTOR_POSITION_PATH =
            "/sys/class/motor/position";

    // Motor calibration data path
    private static final String CAMERA_PERSIST_HALL_CALIBRATION =
            "/mnt/vendor/persist/engineermode/hall_calibration";

    // Motor fallback calibration data
    private static final String HALL_CALIBRATION_DEFAULT =
            "170,170,480,0,0,480,500,0,0,500,1500";

    private CameraMotorController() {
        // This class is not supposed to be instantiated
    }

    private static void writeFile(String path, String data) {
        try (FileWriter fileWriter = new FileWriter(path)) {
            fileWriter.write(data);
        } catch (IOException e) {
            Log.e(TAG, "Failed to write to " + path, e);
        }
    }

    private static String readFile(String path) {
        try (BufferedReader reader = new BufferedReader(new FileReader(path))) {
            return reader.readLine();
        } catch (IOException e) {
            Log.e(TAG, "Failed to read from " + path, e);
            return null;
        }
    }

    public static void calibrate() {
        String calibrationData = HALL_CALIBRATION_DEFAULT;
        try (BufferedReader reader = new BufferedReader(new FileReader(CAMERA_PERSIST_HALL_CALIBRATION))) {
            calibrationData = reader.readLine();
        } catch (IOException e) {
            Log.e(TAG, "Failed to read from " + CAMERA_PERSIST_HALL_CALIBRATION, e);
        }

        writeFile(CAMERA_MOTOR_HALL_CALIBRATION, calibrationData);
    }

    public static boolean getOpen() {
        return readFile(CAMERA_MOTOR_POSITION_PATH) != "1";
    }

    public static void setOpen(boolean enable) {
        writeFile(CAMERA_MOTOR_DIRECTION_PATH, enable ? "1" : "0");
        writeFile(CAMERA_MOTOR_ENABLE_PATH, "1");
    }
}
