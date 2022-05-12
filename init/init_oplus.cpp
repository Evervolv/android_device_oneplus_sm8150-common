/*
 * Copyright (C) 2022 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;

constexpr const char* RO_PROP_SOURCES[] = {
    nullptr,
    "bootimage.",
    "odm.",
    "odm_dlkm.",
    "product.",
    "system.",
    "system_dlkm.",
    "system_ext.",
    "vendor.",
    "vendor_dlkm.",
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

void SetDeviceModel(const char* model) {
    const auto ro_prop_override = [](const char* source, const char* prop, const char* value,
                                     bool product) {
        std::string prop_name = "ro.";

        if (product) prop_name += "product.";
        if (source != nullptr) prop_name += source;
        if (!product) prop_name += "build.";
        prop_name += prop;

        OverrideProperty(prop_name.c_str(), value);
    };

    for (const auto& source : RO_PROP_SOURCES) {
        ro_prop_override(source, "model", model, true);
    }
}

void vendor_load_properties() {
    auto project = std::stoi(GetProperty("ro.boot.prjname", ""));
    int model_num = -1;
    switch (project) {
        case 18825: // 7 Pro 5g Sprint
            model_num = 1925;
            break;
        case 18827: // 7 Pro 5g
            model_num = 1920;
            break;
        default:
            auto rf_version = std::stoi(GetProperty("ro.boot.rf_version", ""));
            switch (rf_version) {
                case 1: // China
                    model_num = 1900;
                    break;
                case 2: // T-Mobile
                    model_num = 1915;
                    break;
                case 3: // India
                    model_num = 1901;
                    break;
                case 4: // Europe
                    model_num = 1903;
                    break;
                default: // Global / US Unlocked
                    model_num = 1907;
                    break;
            }
            if (project >= 18865) {
                model_num += 10;
            }
            break;
    }

    if (model_num == -1) {
        return;
    }

    std::string model = project >= 18865 ? "HD" : "GM";
    model += std::to_string(model_num);
    SetDeviceModel(model.c_str());
}
