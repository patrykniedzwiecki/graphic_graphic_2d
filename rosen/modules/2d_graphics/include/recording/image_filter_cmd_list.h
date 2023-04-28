/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef IMAGE_FILTER_CMD_LILST_H
#define IMAGE_FILTER_CMD_LILST_H

#include "effect/image_filter.h"
#include "recording/cmd_list.h"

namespace OHOS {
namespace Rosen {
namespace Drawing {
class ImageFilterCmdList : public CmdList {
public:
    ImageFilterCmdList() = default;
    ~ImageFilterCmdList() override = default;

    /*
     * @brief       Creates a ImageFilterCmdList with contiguous buffers.
     * @param data  A contiguous buffers.
     */
    static std::shared_ptr<ImageFilterCmdList> CreateFromData(const CmdListData& data);

    /*
     * @brief  Creates a ImageFilter by the ImageFilterCmdList playback operation.
     */
    std::shared_ptr<ImageFilter> Playback() const;
};

/* OpItem */
class ImageFilterOpItem : public OpItem {
public:
    ImageFilterOpItem(uint32_t type) : OpItem(type) {}
    ~ImageFilterOpItem() = default;

    enum Type : uint32_t {
        OPITEM_HEAD = 0, // OPITEM_HEAD must be 0
        CREATE_BLUR,
        CREATE_COLOR_FILTER,
        CREATE_OFFSET,
        CREATE_ARITHMETIC,
        CREATE_COMPOSE,
    };
};

class CreateBlurImageFilterOpItem : public ImageFilterOpItem {
public:
    CreateBlurImageFilterOpItem(scalar sigmaX, scalar sigmaY, TileMode mode, const CmdListSiteInfo& input);
    ~CreateBlurImageFilterOpItem() = default;

    std::shared_ptr<ImageFilter> Playback(const MemAllocator& allocator) const;
private:
    scalar sigmaX_;
    scalar sigmaY_;
    TileMode mode_;
    CmdListSiteInfo input_;
};
} // namespace Drawing
} // namespace Rosen
} // namespace OHOS
#endif