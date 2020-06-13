/*
 *
 *  Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *  Not a Contribution, Apache license notifications and license are retained
 *  for attribution purposes only.
 *
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
int32_t property_get_int32(const char *key, int32_t default_value);
#ifdef __cplusplus
}
#endif

static inline const char* BtmGetDefaultName()
{
    int32_t project_name = property_get_int32("ro.boot.project_name", 0);
    switch (project_name) {
        case 18827:
            /* 5G Europe */
            return "OnePlus 7 Pro";
        case 18831:
            /* T-Mobile */
            return "OnePlus 7 Pro";
        case 19861:
            /* T-Mobile 5G McLaren */
            return "OnePlus 7T Pro";
        default:
            if (project_name != 18857 && project_name != 18865) {
                return project_name >= 18865 ?
                        "OnePlus 7T Pro" : "OnePlus 7 Pro";
            }
            return project_name >= 18865 ?
                    "OnePlus 7T" : "OnePlus 7";
    }
}

#define BTM_DEF_LOCAL_NAME BtmGetDefaultName()

// Disables read remote device feature
#define BTM_WBS_INCLUDED TRUE
#define BTIF_HF_WBS_PREFERRED TRUE

#define BLE_VND_INCLUDED   TRUE
// skips conn update at conn completion
#define BT_CLEAN_TURN_ON_DISABLED 1
#endif

