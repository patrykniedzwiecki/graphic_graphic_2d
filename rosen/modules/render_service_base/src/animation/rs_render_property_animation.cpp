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

#include "animation/rs_render_property_animation.h"

#include "modifier/rs_render_property.h"
#include "pipeline/rs_canvas_render_node.h"
#include "platform/common/rs_log.h"
#include "transaction/rs_marshalling_helper.h"

namespace OHOS {
namespace Rosen {
RSRenderPropertyAnimation::RSRenderPropertyAnimation(AnimationId id, const PropertyId& propertyId,
    const std::shared_ptr<RSRenderPropertyBase>& originValue) : RSRenderAnimation(id), propertyId_(propertyId),
    originValue_(originValue), lastValue_(originValue->GetValue())
{}

PropertyId RSRenderPropertyAnimation::GetPropertyId() const
{
    return propertyId_;
}

void RSRenderPropertyAnimation::SetAdditive(bool isAdditive)
{
    if (IsStarted()) {
        ROSEN_LOGE("Failed to set additive, animation has started!");
        return;
    }

    isAdditive_ = isAdditive;
}

bool RSRenderPropertyAnimation::GetAdditive()
{
    return isAdditive_;
}

void RSRenderPropertyAnimation::AttachRenderProperty(const std::shared_ptr<RSRenderPropertyBase>& property)
{
    property_ = property;
    if (property_ != nullptr && originValue_ != nullptr) {
        property_->SetPropertyType(originValue_->GetPropertyType());
    }
}

#ifdef ROSEN_OHOS
bool RSRenderPropertyAnimation::Marshalling(Parcel& parcel) const
{
    if (!RSRenderAnimation::Marshalling(parcel)) {
        ROSEN_LOGE("RSRenderPropertyAnimation::Marshalling, RenderAnimation failed");
        return false;
    }
    if (!parcel.WriteUint64(propertyId_)) {
        ROSEN_LOGE("RSRenderPropertyAnimation::Marshalling, write PropertyId failed");
        return false;
    }
    if (!(RSMarshallingHelper::Marshalling(parcel, isAdditive_) &&
            RSRenderPropertyBase::Marshalling(parcel, originValue_))) {
        ROSEN_LOGE("RSRenderPropertyAnimation::Marshalling, write value failed");
        return false;
    }
    return true;
}

bool RSRenderPropertyAnimation::ParseParam(Parcel& parcel)
{
    if (!RSRenderAnimation::ParseParam(parcel)) {
        ROSEN_LOGE("RSRenderPropertyAnimation::ParseParam, RenderAnimation failed");
        return false;
    }

    if (!(parcel.ReadUint64(propertyId_) && RSMarshallingHelper::Unmarshalling(parcel, isAdditive_))) {
        ROSEN_LOGE("RSRenderPropertyAnimation::ParseParam, Unmarshalling failed");
        return false;
    }
    if (!RSRenderPropertyBase::Unmarshalling(parcel, originValue_)) {
        return false;
    }
    lastValue_ = originValue_->GetValue();

    return true;
}
#endif

void RSRenderPropertyAnimation::SetPropertyValue(const std::shared_ptr<RSRenderPropertyBase>& value)
{
    if (property_ != nullptr) {
        property_->SetValue(value);
    }
}

const std::shared_ptr<RSRenderPropertyBase> RSRenderPropertyAnimation::GetPropertyValue() const
{
    if (property_ != nullptr) {
        return property_->GetValue();
    }

    return lastValue_->GetValue();
}

const std::shared_ptr<RSRenderPropertyBase>& RSRenderPropertyAnimation::GetOriginValue() const
{
    return originValue_;
}

const std::shared_ptr<RSRenderPropertyBase>& RSRenderPropertyAnimation::GetLastValue() const
{
    return lastValue_;
}

void RSRenderPropertyAnimation::SetAnimationValue(const std::shared_ptr<RSRenderPropertyBase>& value)
{
    SetPropertyValue(GetAnimationValue(value));
}

const std::shared_ptr<RSRenderPropertyBase> RSRenderPropertyAnimation::GetAnimationValue(
    const std::shared_ptr<RSRenderPropertyBase>& value)
{
    std::shared_ptr<RSRenderPropertyBase> animationValue;
    if (GetAdditive()) {
        animationValue = GetPropertyValue() + value - lastValue_;
    } else {
        animationValue = value;
    }

    lastValue_ = value;
    return animationValue;
}

void RSRenderPropertyAnimation::OnRemoveOnCompletion()
{
    std::shared_ptr<RSRenderPropertyBase> backwardValue;
    if (GetAdditive()) {
        backwardValue = GetPropertyValue() + GetOriginValue() - lastValue_;
    } else {
        backwardValue = GetOriginValue();
    }

    SetPropertyValue(backwardValue);
}

void RSRenderPropertyAnimation::WriteAnimationValueToLog(const std::shared_ptr<RSRenderPropertyBase>& value)
{
    auto node = GetTarget();
    if (node == nullptr) {
        return;
    }

    UpdateNeedWriteLog(node->GetId());
    if (needWriteToLog_) {
        animationLog_->WriteAnimationValueToLog(value, propertyId_, node->GetId());
    }
}

void RSRenderPropertyAnimation::WriteAnimationInfoToLog(const std::shared_ptr<RSRenderPropertyBase>& startValue,
    const std::shared_ptr<RSRenderPropertyBase>& endValue)
{
    if (hasWriteInfo_) {
        return;
    }

    hasWriteInfo_ = true;
    auto node = GetTarget();
    if (node == nullptr) {
        return;
    }

    UpdateNeedWriteLog(node->GetId());
    if (needWriteToLog_) {
        animationLog_->WriteAnimationInfoToLog(propertyId_, GetAnimationId(), startValue, endValue);
    }
}

void RSRenderPropertyAnimation::UpdateNeedWriteLog(const NodeId id)
{
    if (!hasUpdateNeedWriteLog_) {
        hasUpdateNeedWriteLog_ = true;
        needWriteToLog_ = animationLog_->IsNeedWriteLog(propertyId_, id);
    }
}
} // namespace Rosen
} // namespace OHOS
