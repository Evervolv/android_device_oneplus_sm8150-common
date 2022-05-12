/*
   Copyright (C) 2017-2018 The Android Open Source Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;

constexpr const char* RO_PROP_SOURCES[] = {
        nullptr, "product.", "product_services.", "odm.", "vendor.", "system.", "bootimage.",
};

constexpr const char* BUILD_DEVICE[] = {
        "OnePlus7ProNR",
        "OnePlus7ProTMO",
        "OnePlus7TProNR",
};

constexpr const char* BUILD_DESCRIPTION[] = {
        "OnePlus7ProNR-user 10 QKQ1.190716.003 2109160130 release-keys",
        "OnePlus7ProTMO-user 11 RKQ1.201022.002 2204011431 release-keys",
        "OnePlus7TProNR-user 11 RKQ1.201022.002 2110121803 release-keys",
};

constexpr const char* BUILD_FINGERPRINT[] = {
        "OnePlus/OnePlus7ProNR_EEA/OnePlus7ProNR:10/QKQ1.190716.003/2109160130:user/release-keys",
        "OnePlus/OnePlus7ProTMO/OnePlus7ProTMO:11/RKQ1.201022.002/2204011431:user/release-keys",
        "OnePlus/OnePlus7TProNR/OnePlus7TProNR:11/RKQ1.201022.002/2110121803:user/release-key",
};

constexpr const char* BUILD_SECURITY_PATCH[] = {
        "2021-09-01",
        "2022-04-01",
        "2021-10-01",
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

void SetDeviceProperties(const char* model, int id) {
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

    if (id < 0) {
        return;
    }

    for (const auto& source : RO_PROP_SOURCES) {
        ro_prop_override(source, "device", BUILD_DEVICE[id], true);
        ro_prop_override(source, "fingerprint", BUILD_FINGERPRINT[id], false);
        ro_prop_override(source, "version.security_patch", BUILD_SECURITY_PATCH[id], false);
    }
    ro_prop_override(nullptr, "product", BUILD_DEVICE[id], false);
    ro_prop_override(nullptr, "description", BUILD_DESCRIPTION[id], false);
}

void vendor_load_properties() {
    auto project = std::stoi(GetProperty("ro.boot.prjname", ""));
    auto rf_version = std::stoi(GetProperty("ro.boot.rf_version", ""));

    bool is_pro = project != 18857 && project != 18865;

    std::string model = project >= 18865 ? "HD" : "GM";
    int id = -1;
    if (rf_version == 1 /* China*/) {
        model += is_pro ? "1910" : "1900";
    } else if (rf_version == 2 /* T-Mobile */) {
        if (project == 18821) {
            id = 1;
            model += "1915";
        } else if (project == 19861) {
            id = 2;
            model += "1925";
        }
    } else if (rf_version == 3 /* India*/) {
        model += is_pro ? "1911" : "1901";
    } else if (rf_version == 4 /* Europe */) {
        if (project == 18827) {
            id = 0;
            model += "1920";
        } else {
            model += is_pro ? "1913" : "1903";
        }
    } else {
        /* Global / US Unlocked */
        model += is_pro ? "1917" : "1907";
    }

    SetDeviceProperties(model.c_str(), id);
}
