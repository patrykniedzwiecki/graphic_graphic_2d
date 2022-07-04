/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hdiscreen_fuzzer.h"

#include "hdi_screen.h"
using namespace OHOS::Rosen;

namespace OHOS {
    namespace {
        constexpr size_t STR_LEN = 10;
        constexpr char STR_END_CHAR = '\0';
        const uint8_t* data_ = nullptr;
        size_t size_ = 0;
        size_t pos;
    }

    /*
    * describe: get data from outside untrusted data(data_) which size is according to sizeof(T)
    * tips: only support basic type
    */
    template<class T>
    T GetData()
    {
        T object {};
        size_t objectSize = sizeof(object);
        if (data_ == nullptr || objectSize > size_ - pos) {
            return object;
        }
        std::memcpy(&object, data_ + pos, objectSize);
        pos += objectSize;
        return object;
    }

    /*
    * get a string from data_
    */
    std::string GetStringFromData(int strlen)
    {
        char cstr[strlen];
        cstr[strlen-1] = STR_END_CHAR;
        for (int i = 0; i < strlen-1; i++) {
            cstr[i] = GetData<char>();
        }
        std::string str(cstr);
        return str;
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        if (data == nullptr || size < 0) {
            return false;
        }

        // initialize
        data_ = data;
        size_ = size;
        pos = 0;

        // get data
        uint32_t screenId = GetData<uint32_t>();
        uint32_t modeId = GetData<uint32_t>();
        DispPowerStatus status = GetData<DispPowerStatus>();
        uint32_t level = GetData<uint32_t>();
        bool enabled = GetData<bool>();
        ColorGamut gamut = GetData<ColorGamut>();
        GamutMap gamutMap = GetData<GamutMap>();
        float matrix = GetData<float>();
        uint32_t sequence = GetData<uint32_t>();
        uint64_t ns = GetData<uint64_t>();
        void* dt = static_cast<void*>(GetStringFromData(STR_LEN).data());

        // test
        std::unique_ptr<HdiScreen> hdiScreen = HdiScreen::CreateHdiScreen(screenId);
        hdiScreen->Init();
        hdiScreen->SetScreenMode(modeId);
        hdiScreen->SetScreenPowerStatus(status);
        hdiScreen->SetScreenBacklight(level);
        hdiScreen->SetScreenVsyncEnabled(enabled);
        hdiScreen->SetScreenColorGamut(gamut);
        hdiScreen->SetScreenGamutMap(gamutMap);
        hdiScreen->SetScreenColorTransform(&matrix);
        hdiScreen->OnVsync(sequence, ns, dt);
        
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

