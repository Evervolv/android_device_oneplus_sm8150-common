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

constexpr struct {
    const char* device;
    const char* product;
    const char* description;
    const char* fingerprint;
} BUILD_INFO[] = {
    {"OP7ProNRSpr", "OP7ProNRSpr", "OP7ProNRSpr-user 10 QKQ1.190716.003 2204192115 release-keys", "OnePlus/OP7ProNRSpr/OP7ProNRSpr:10/QKQ1.190716.003/2204192115:user/release-keys"},
    {"OnePlus7ProNR", "OnePlus7ProNR_EEA", "OnePlus7ProNR-user 10 QKQ1.190716.003 2109160130 release-keys", "OnePlus/OnePlus7ProNR_EEA/OnePlus7ProNR:10/QKQ1.190716.003/2109160130:user/release-keys"},
    {"OnePlus7ProTMO", "OnePlus7ProTMO", "OnePlus7ProTMO-user 11 RKQ1.201022.002 2204011431 release-keys", "OnePlus/OnePlus7ProTMO/OnePlus7ProTMO:11/RKQ1.201022.002/2204011431:user/release-keys"},
    {"OnePlus7TProNR", "OnePlus7TProNR", "OnePlus7TProNR-user 11 RKQ1.201022.002 2108021431 release-keys", "OnePlus/OnePlus7TProNR/OnePlus7TProNR:11/RKQ1.201022.002/2108021431:user/release-keys"}
};

static const std::unordered_map<int, int> region_map = {
    {1, 1900}, {2, 1915}, {3, 1901}, {4, 1903}, {0, 1907}
};

static const std::unordered_map<int, std::pair<int, int>> variant_map = {
    {18825, {1925, 0}}, {18827, {1920, 1}}, {18831, {1915, 2}}, {19861, {1925, 3}}
};

void OverrideProperty(const char* name, const char* value) {
    if (auto* pi = (prop_info*)__system_property_find(name)) {
        __system_property_update(pi, value, strlen(value));
    } else {
        __system_property_add(name, strlen(name), value, strlen(value));
    }
}

void SetDeviceModel(int id, int num, int index) {
    std::string model = (id >= 18865 ? "HD" : "GM") + std::to_string(num);
    
    for (const auto* source : RO_PROP_SOURCES) {
        std::string prefix = "ro." + (source ? std::string(source) : "");
        OverrideProperty((prefix + "product.model").c_str(), model.c_str());
        if (index >= 0 && index < 4) {
            OverrideProperty((prefix + "product.name").c_str(), BUILD_INFO[index].product);
            OverrideProperty((prefix + "product.device").c_str(), BUILD_INFO[index].device);
            OverrideProperty((prefix + "build.fingerprint").c_str(), BUILD_INFO[index].fingerprint);
        }
    }
    if (index >= 0 && index < 4) {
        OverrideProperty("ro.build.product", BUILD_INFO[index].product);
        OverrideProperty("ro.build.description", BUILD_INFO[index].description);
    }
}

void vendor_load_properties() {
    if (access("/system/bin/recovery", F_OK) == 0) return;

    int project = std::stoi(GetProperty("ro.boot.prjname", "0"));
    if (auto it = variant_map.find(project); it != variant_map.end()) {
        SetDeviceModel(project, it->second.first, it->second.second);
        return;
    }

    int region = std::stoi(GetProperty("ro.boot.rf_version", "0"));
    int model_num = region_map.at(region);
    if (project >= 18865) model_num += 10;
    SetDeviceModel(project, model_num, -1);
}

