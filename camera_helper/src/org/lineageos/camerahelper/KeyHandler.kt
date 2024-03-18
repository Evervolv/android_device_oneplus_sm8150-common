/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.camerahelper

import android.app.AlertDialog
import android.content.Context
import android.content.Intent
import android.os.Handler
import android.os.Looper
import android.view.KeyEvent
import android.view.WindowManager
import com.android.internal.os.DeviceKeyHandler

class KeyHandler(private val context: Context) : DeviceKeyHandler {

    override fun handleKeyEvent(event: KeyEvent): KeyEvent? {
        when (event.scanCode) {
            MOTOR_EVENT_MANUAL_TO_DOWN,
            MOTOR_EVENT_UP_ABNORMAL,
            MOTOR_EVENT_DOWN_ABNORMAL -> {
                if (event.action != KeyEvent.ACTION_DOWN) {
                    return null
                }

                if (event.scanCode == MOTOR_EVENT_MANUAL_TO_DOWN) {
                    closeAllApps()
                }

                Handler(Looper.getMainLooper()).post {
                    showCameraMotorWarning(event.scanCode)?.apply {
                        window?.setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY)
                        setCanceledOnTouchOutside(false)
                        show()
                    }
                }
                return null
            }
            else -> return event
        }
    }

    private fun closeAllApps() {
        // Go back to home to close all camera apps first
        val intent = Intent(Intent.ACTION_MAIN).apply {
            addCategory(Intent.CATEGORY_HOME)
            setFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
        }
        context.startActivity(intent)
    }

    private fun showCameraMotorWarning(scanCode: Int): AlertDialog? {
        val packageContext = runCatching {
            context.createPackageContext(
                KeyHandler::class.java.`package`!!.name,
                0
            )
        }.getOrElse { return null }

        return when (scanCode) {
            MOTOR_EVENT_MANUAL_TO_DOWN -> {
                AlertDialog.Builder(packageContext).apply {
                    setTitle(R.string.warning)
                    setMessage(R.string.motor_press_message)
                    setPositiveButton(android.R.string.ok, null)
                }.create()
            }
            MOTOR_EVENT_UP_ABNORMAL -> {
                AlertDialog.Builder(packageContext).apply {
                    setTitle(R.string.warning)
                    setMessage(R.string.motor_cannot_go_up_message)
                    setNegativeButton(R.string.retry) { _, _ ->
                        CameraMotorController.setMotorDirection("1")
                    }
                    setPositiveButton(R.string.close) { _, _ ->
                        CameraMotorController.setMotorDirection("0")
                        closeAllApps()
                    }
                }.create()
            }
            MOTOR_EVENT_DOWN_ABNORMAL -> {
                AlertDialog.Builder(packageContext).apply {
                    setMessage(R.string.motor_cannot_go_down_message)
                    setPositiveButton(R.string.retry) { _, _ ->
                        CameraMotorController.setMotorDirection("0")
                   }
                }.create()
                }
            else -> null
        }
    }


    companion object {
        // Camera motor event key codes
        private const val MOTOR_EVENT_MANUAL_TO_DOWN = 184
        private const val MOTOR_EVENT_UP_ABNORMAL = 186
        private const val MOTOR_EVENT_DOWN_ABNORMAL = 189
    }
}
