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
        "OnePlus7",
        "OnePlus7Pro",
        "OnePlus7T",
        "OnePlus7TPro",
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

    char variant[7];
    strcpy (variant, id > 1 ? "HD" : "GM");
    strcat (variant, model);

    for (const auto& source : RO_PROP_SOURCES) {
        ro_prop_override(source, "device", BUILD_DEVICE[id], true);
        ro_prop_override(source, "model", variant, true);
    }
    ro_prop_override(nullptr, "product", BUILD_DEVICE[id], false);
}

void vendor_load_properties() {
    auto project = std::stoi(GetProperty("ro.boot.prjname", ""));

    bool is_pro = project != 18857 && project != 18865;
    int id = project >= 18865 ? 2 : 0;
    if (is_pro) {
        id = project >= 18865 ? 3 : 1;
    }

    switch (project){
        case 18827:
            /* 5G Europe */
            SetDeviceProperties("1920", id);
            break;
        case 18831:
            /* T-Mobile */
            SetDeviceProperties("1915", id);
            break;
        case 19861:
            /* T-Mobile 5G McLaren */
            SetDeviceProperties("1925", id);
            break;
        default:
            auto rf_version = std::stoi(GetProperty("ro.boot.rf_version", ""));
            switch (rf_version){
                case 1:
                    /* China*/
                    SetDeviceProperties(is_pro ? "1910" : "1900", id);
                    break;
                case 3:
                    /* India*/
                    SetDeviceProperties(is_pro ? "1911" : "1901", id);
                    break;
                case 4:
                    /* Europe */
                    SetDeviceProperties(is_pro ? "1913" : "1903", id);
                    break;
                default:
                    /* Global / US Unlocked */
                    SetDeviceProperties(is_pro ? "1917" : "1907", id);
                    break;
            }
        }
}
