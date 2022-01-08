/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef SKIA_COLOR_SPACE_H
#define SKIA_COLOR_SPACE_H

#include "include/core/SkColorSpace.h"

#include "impl_interface/color_space_impl.h"

namespace OHOS {
namespace Rosen {
namespace Drawing {
class SkiaColorSpace : public ColorSpaceImpl {
public:
    static inline constexpr AdapterType TYPE = AdapterType::SKIA_ADAPTER;
    SkiaColorSpace() noexcept;
    ~SkiaColorSpace() {};
    AdapterType GetType() const override { return AdapterType::SKIA_ADAPTER; }

    void InitWithSRGB() override;
    void InitWithSRGBLinear() override;
    sk_sp<SkColorSpace> GetColorSpace() const;
private:
    sk_sp<SkColorSpace> colorSpace_;
};
}
}
}
#endif