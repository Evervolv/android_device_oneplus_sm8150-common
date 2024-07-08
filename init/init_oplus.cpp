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

constexpr const char* BUILD_DEVICE[] = {
    "OP7ProNRSpr",
    "OnePlus7ProNR",
    "OnePlus7ProTMO",
    "OnePlus7TProNR",
};

constexpr const char* BUILD_PRODUCT[] = {
    "OP7ProNRSpr",
    "OnePlus7ProNR_EEA",
    "OnePlus7ProTMO",
    "OnePlus7TProNR",
};

constexpr const char* BUILD_DESCRIPTION[] = {
    "OP7ProNRSpr-user 10 QKQ1.190716.003 2204192115 release-keys",
    "OnePlus7ProNR-user 10 QKQ1.190716.003 2109160130 release-keys",
    "OnePlus7ProTMO-user 11 RKQ1.201022.002 2204011431 release-keys",
    "OnePlus7TProNR-user 11 RKQ1.201022.002 2108021431 release-keys",
};

constexpr const char* BUILD_FINGERPRINT[] = {
    "OnePlus/OP7ProNRSpr/OP7ProNRSpr:10/QKQ1.190716.003/2204192115:user/release-keys",
    "OnePlus/OnePlus7ProNR_EEA/OnePlus7ProNR:10/QKQ1.190716.003/2109160130:user/release-keys",
    "OnePlus/OnePlus7ProTMO/OnePlus7ProTMO:11/RKQ1.201022.002/2204011431:user/release-keys",
    "OnePlus/OnePlus7TProNR/OnePlus7TProNR:11/RKQ1.201022.002/2108021431:user/release-keys",
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

void SetDeviceModel(int project, int model_num, int carrier) {
    const auto ro_prop_override = [](const char* source, const char* prop, const char* value,
                                     bool product) {
        std::string prop_name = "ro.";

        if (product) prop_name += "product.";
        if (source != nullptr) prop_name += source;
        if (!product) prop_name += "build.";
        prop_name += prop;

        OverrideProperty(prop_name.c_str(), value);
    };

    std::string model = project >= 18865 ? "HD" : "GM";
    model += std::to_string(model_num);

    for (const auto& source : RO_PROP_SOURCES) {
        ro_prop_override(source, "model", model.c_str(), true);
    }

    if (carrier < 0 || carrier > 3) {
        return;
    }

    for (const auto& source : RO_PROP_SOURCES) {
        ro_prop_override(source, "name", BUILD_PRODUCT[carrier], true);
        ro_prop_override(source, "device", BUILD_DEVICE[carrier], true);
        ro_prop_override(source, "fingerprint", BUILD_FINGERPRINT[carrier], false);
    }
    ro_prop_override(nullptr, "product", BUILD_PRODUCT[carrier], false);
    ro_prop_override(nullptr, "description", BUILD_DESCRIPTION[carrier], false);
}

void vendor_load_properties() {
    if (access("/system/bin/recovery", F_OK) == 0) {
        return;
    }

    auto project = std::stoi(GetProperty("ro.boot.prjname", ""));
    auto rf_version = std::stoi(GetProperty("ro.boot.rf_version", ""));
    int carrier = -1;
    int model_num = -1;
    switch (project) {
        case 18825: // 7 Pro 5g Sprint
            model_num = 1925;
            carrier = 0;
            break;
        case 18827: // 7 Pro 5g
            model_num = 1920;
            carrier = 1;
            break;
        case 18821: // 7 Pro
            if (rf_version == 2) {
                model_num = 1915;
                carrier = 2;
            }
            break;
        case 18831: // 7 Pro T-Mobile
            model_num = 1915;
            carrier = 2;
            break;
		case 19861: // 7T Pro 5g
            model_num = 1925;
            carrier = 3;
            break;
        default:
            break;
    }

    if (model_num != -1 && carrier != -1) {
        SetDeviceModel(project, model_num, carrier);
        return;
    }

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

    SetDeviceModel(project, model_num, carrier);
}
