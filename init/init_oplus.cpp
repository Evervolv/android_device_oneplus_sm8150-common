/*
 * Copyright (C) 2022 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <unordered_map>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;

constexpr const char* RO_PROP_SOURCES[] = {
    nullptr, "bootimage.", "odm.", "odm_dlkm.", "product.", "system.", "system_dlkm.", "system_ext.", "vendor.", "vendor_dlkm."
};

static const std::unordered_map<int, int> region_map = {
    {1, 1900}, {2, 1915}, {3, 1901}, {4, 1903}, {0, 1907}
};

void OverrideProperty(const char* name, const char* value) {
    if (auto* pi = (prop_info*)__system_property_find(name)) {
        __system_property_update(pi, value, strlen(value));
    } else {
        __system_property_add(name, strlen(name), value, strlen(value));
    }
}

void SetDeviceModel(int id, int num) {
    std::string model = (id >= 18865 ? "HD" : "GM") + std::to_string(num);
    
    for (const auto* source : RO_PROP_SOURCES) {
        std::string prefix = "ro." + (source ? std::string(source) : "");
        OverrideProperty((prefix + "product.model").c_str(), model.c_str());
    }
}

void vendor_load_properties() {
    if (access("/system/bin/recovery", F_OK) == 0) return;

    int project = std::stoi(GetProperty("ro.boot.prjname", "0"));
    int region = std::stoi(GetProperty("ro.boot.rf_version", "0"));
    int model_num = region_map.at(region);
    if (project >= 18865) model_num += 10;
    SetDeviceModel(project, model_num);
}

