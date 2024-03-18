/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.camerahelper

import android.app.AlertDialog
import android.content.Context
import android.content.DialogInterface
import android.content.Intent
import android.os.Handler
import android.os.Looper
import android.view.KeyEvent
import android.view.WindowManager
import com.android.internal.os.DeviceKeyHandler
import java.util.concurrent.Executors

class KeyHandler(private val context: Context) : DeviceKeyHandler {
    private val executorService = Executors.newSingleThreadExecutor()
    private val packageContext = context.createPackageContext(
        KeyHandler::class.java.getPackage()!!.name, 0
    )

    override fun handleKeyEvent(event: KeyEvent): KeyEvent? {
        when (event.scanCode) {
            MOTOR_EVENT_MANUAL_TO_DOWN,
            MOTOR_EVENT_UP_ABNORMAL,
            MOTOR_EVENT_DOWN_ABNORMAL ->
                if (event.action != KeyEvent.ACTION_DOWN) {
                    return null
                }
            else -> return event
        }

        if (event.scanCode == MOTOR_EVENT_MANUAL_TO_DOWN) {
            closeAllApps()
        }

        executorService.submit {
            val dialog = AlertDialog.Builder(
                packageContext
            ).apply {
                setTitle(R.string.warning)
                when (event.scanCode) {
                    MOTOR_EVENT_MANUAL_TO_DOWN -> {
                        setMessage(R.string.motor_press_message)
                        setPositiveButton(android.R.string.ok, null)
                    }
                    MOTOR_EVENT_UP_ABNORMAL -> {
                        setMessage(R.string.motor_cannot_go_up_message)
                        setNegativeButton(
                            R.string.retry
                        ) { _: DialogInterface?, _: Int ->
                            CameraMotorController.setMotorDirection("1")
                        }
                        setPositiveButton(
                            R.string.close
                        ) { _: DialogInterface?, _: Int ->
                            CameraMotorController.setMotorDirection("0")
                            closeAllApps()
                        }
                    }
                    MOTOR_EVENT_DOWN_ABNORMAL -> {
                        setMessage(R.string.motor_cannot_go_down_message)
                        setPositiveButton(
                            R.string.retry
                        ) { _: DialogInterface?, _: Int ->
                            // Close the camera
                            CameraMotorController.setMotorDirection("0")
                        }
                    }
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
        return null
    }

    private fun closeAllApps() {
        // Go back to home to close all camera apps first
        context.startActivity(Intent(Intent.ACTION_MAIN).apply {
            addCategory(Intent.CATEGORY_HOME)
            setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        })
    }

    companion object {
        // Camera motor event key codes
        private const val MOTOR_EVENT_MANUAL_TO_DOWN = 184
        private const val MOTOR_EVENT_UP_ABNORMAL = 186
        private const val MOTOR_EVENT_DOWN_ABNORMAL = 189
    }
}
