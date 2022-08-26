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

#ifndef OHOS_JS_COLOR_SPACE_H
#define OHOS_JS_COLOR_SPACE_H

#include <memory>

#include "color_space.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace ColorManager {
void BindFunctions(NativeEngine& engine, NativeObject* object);
class JsColorSpace final {
public:
    explicit JsColorSpace(const std::shared_ptr<ColorSpace>& colorSpace) : colorSpaceToken_(colorSpace) {};
    ~JsColorSpace() {};
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* GetColorSpaceName(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetWhitePoint(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetGamma(NativeEngine* engine, NativeCallbackInfo* info);
    inline const std::shared_ptr<ColorSpace>& GetColorSpaceToken() const
    {
        return colorSpaceToken_;
    }

private:
    NativeValue* OnGetColorSpaceName(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetWhitePoint(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetGamma(NativeEngine& engine, NativeCallbackInfo& info);

    std::shared_ptr<ColorSpace> colorSpaceToken_;
};
}  // namespace ColorManager
}  // namespace OHOS

#endif // OHOS_JS_COLOR_SPACE_H