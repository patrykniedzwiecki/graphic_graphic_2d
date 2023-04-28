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

#include "recording/recording_shader_effect.h"

#include "image/image.h"
#include "utils/log.h"

namespace OHOS {
namespace Rosen {
namespace Drawing {
RecordingShaderEffect::RecordingShaderEffect() noexcept : cmdList_(std::make_shared<ShaderEffectCmdList>()) {}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreateColorShader(ColorQuad color)
{
    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    shaderEffect->GetCmdList()->AddOp<CreateColorShaderOpItem>(color);
    return shaderEffect;
}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreateBlendShader(
    const ShaderEffect& dst, const ShaderEffect& src, BlendMode mode)
{
    auto dstCmdList = static_cast<const RecordingShaderEffect&>(dst).GetCmdList();
    auto srcCmdList = static_cast<const RecordingShaderEffect&>(src).GetCmdList();
    if (dstCmdList == nullptr || srcCmdList == nullptr) {
        LOGE("RecordingShaderEffect::CreateBlendShader, dst or src is valid!");
        return nullptr;
    }

    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    auto cmdData = dstCmdList->GetData();
    auto offset = shaderEffect->GetCmdList()->AddCmdListData(cmdData);
    CmdListSiteInfo dstData(offset, cmdData.second);

    cmdData = srcCmdList->GetData();
    offset = shaderEffect->GetCmdList()->AddCmdListData(cmdData);
    CmdListSiteInfo srcData(offset, cmdData.second);

    shaderEffect->GetCmdList()->AddOp<CreateBlendShaderOpItem>(dstData, srcData, mode);
    return shaderEffect;
}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreateImageShader(
    const Image& image, TileMode tileX, TileMode tileY, const SamplingOptions& sampling, const Matrix& matrix)
{
    // Use deep copy to implement, and then use shared memory optimization
    auto data = image.Serialize();
    if (data->GetSize() == 0) {
        LOGE("RecordingShaderEffect::CreateImageShader, image is valid!");
        return nullptr;
    }

    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    auto offset = shaderEffect->GetCmdList()->AddLargeObject(LargeObjectData(data->GetData(), data->GetSize()));
    LargeObjectInfo info(offset, data->GetSize());

    shaderEffect->GetCmdList()->AddOp<CreateImageShaderOpItem>(info, tileX, tileY, sampling, matrix);
    return shaderEffect;
}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreatePictureShader(const Picture& picture,
    TileMode tileX, TileMode tileY, FilterMode mode, const Matrix& matrix, const Rect& rect)
{
    // Use deep copy to implement, and then use shared memory optimization
    auto data = picture.Serialize();
    if (data->GetSize() == 0) {
        LOGE("RecordingShaderEffect::CreatePictureShader, picture is valid!");
        return nullptr;
    }

    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    auto offset = shaderEffect->GetCmdList()->AddLargeObject(LargeObjectData(data->GetData(), data->GetSize()));
    LargeObjectInfo info(offset, data->GetSize());

    shaderEffect->GetCmdList()->AddOp<CreatePictureShaderOpItem>(info, tileX, tileY, mode, matrix, rect);
    return shaderEffect;
}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreateLinearGradient(const Point& startPt,
    const Point& endPt, const std::vector<ColorQuad>& colors, const std::vector<scalar>& pos, TileMode mode)
{
    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    std::pair<int, size_t> colorsData(0, 0);
    if (!colors.empty()) {
        const void* data = static_cast<const void*>(colors.data());
        size_t size = colors.size() * sizeof(ColorQuad);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        colorsData.first = offset;
        colorsData.second = size;
    }

    std::pair<int, size_t> posData(0, 0);
    if (!pos.empty()) {
        const void* data = static_cast<const void*>(pos.data());
        size_t size = pos.size() * sizeof(scalar);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        posData.first = offset;
        posData.second = size;
    }

    shaderEffect->GetCmdList()->AddOp<CreateLinearGradientOpItem>(startPt, endPt, colorsData, posData, mode);
    return shaderEffect;
}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreateRadialGradient(const Point& centerPt,
    scalar radius, const std::vector<ColorQuad>& colors, const std::vector<scalar>& pos, TileMode mode)
{
    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    std::pair<int, size_t> colorsData(0, 0);
    if (!colors.empty()) {
        const void* data = static_cast<const void*>(colors.data());
        size_t size = colors.size() * sizeof(ColorQuad);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        colorsData.first = offset;
        colorsData.second = size;
    }

    std::pair<int, size_t> posData(0, 0);
    if (!pos.empty()) {
        const void* data = static_cast<const void*>(pos.data());
        size_t size = pos.size() * sizeof(scalar);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        posData.first = offset;
        posData.second = size;
    }

    shaderEffect->GetCmdList()->AddOp<CreateRadialGradientOpItem>(centerPt, radius, colorsData, posData, mode);
    return shaderEffect;
}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreateTwoPointConical(
    const Point& startPt, scalar startRadius, const Point& endPt, scalar endRadius,
    const std::vector<ColorQuad>& colors, const std::vector<scalar>& pos, TileMode mode)
{
    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    std::pair<int, size_t> colorsData(0, 0);
    if (!colors.empty()) {
        const void* data = static_cast<const void*>(colors.data());
        size_t size = colors.size() * sizeof(ColorQuad);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        colorsData.first = offset;
        colorsData.second = size;
    }

    std::pair<int, size_t> posData(0, 0);
    if (!pos.empty()) {
        const void* data = static_cast<const void*>(pos.data());
        size_t size = pos.size() * sizeof(scalar);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        posData.first = offset;
        posData.second = size;
    }

    shaderEffect->GetCmdList()->AddOp<CreateTwoPointConicalOpItem>(
        startPt, startRadius, endPt, endRadius, colorsData, posData, mode);
    return shaderEffect;
}

std::shared_ptr<RecordingShaderEffect> RecordingShaderEffect::CreateSweepGradient(
    const Point& centerPt, const std::vector<ColorQuad>& colors, const std::vector<scalar>& pos,
    TileMode mode, scalar startAngle, scalar endAngle)
{
    auto shaderEffect = std::make_shared<RecordingShaderEffect>();
    std::pair<int, size_t> colorsData(0, 0);
    if (!colors.empty()) {
        const void* data = static_cast<const void*>(colors.data());
        size_t size = colors.size() * sizeof(ColorQuad);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        colorsData.first = offset;
        colorsData.second = size;
    }

    std::pair<int, size_t> posData(0, 0);
    if (!pos.empty()) {
        const void* data = static_cast<const void*>(pos.data());
        size_t size = pos.size() * sizeof(scalar);
        auto offset = shaderEffect->GetCmdList()->AddCmdListData({ data, size });
        posData.first = offset;
        posData.second = size;
    }

    shaderEffect->GetCmdList()->AddOp<CreateSweepGradientOpItem>(
        centerPt, colorsData, posData, mode, startAngle, endAngle);
    return shaderEffect;
}
} // namespace Drawing
} // namespace Rosen
} // namespace OHOS