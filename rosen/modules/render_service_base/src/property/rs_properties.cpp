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

#include "property/rs_properties.h"

#include <algorithm>
#include <securec.h>

#include "platform/common/rs_log.h"
#include "render/rs_filter.h"
#ifdef ROSEN_OHOS
#include "common/rs_obj_abs_geometry.h"
#else
#include "common/rs_obj_geometry.h"
#endif

namespace OHOS {
namespace Rosen {
RSProperties::RSProperties(bool inRenderNode)
{
#ifdef ROSEN_OHOS
    if (inRenderNode) {
        boundsGeo_ = std::make_shared<RSObjAbsGeometry>();
    } else {
        boundsGeo_ = std::make_shared<RSObjGeometry>();
    }
#else
    boundsGeo_ = std::make_shared<RSObjGeometry>();
#endif
    frameGeo_ = std::make_shared<RSObjGeometry>();
}

RSProperties::~RSProperties() {}

void RSProperties::SetBounds(Vector4f bounds)
{
    boundsGeo_->SetRect(bounds.x_, bounds.y_, bounds.z_, bounds.w_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetBoundsSize(Vector2f size)
{
    boundsGeo_->SetSize(size.x_, size.y_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetBoundsWidth(float width)
{
    boundsGeo_->SetWidth(width);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetBoundsHeight(float height)
{
    boundsGeo_->SetHeight(height);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetBoundsPosition(Vector2f position)
{
    boundsGeo_->SetPosition(position.x_, position.y_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetBoundsPositionX(float positionX)
{
    boundsGeo_->SetX(positionX);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetBoundsPositionY(float positionY)
{
    boundsGeo_->SetY(positionY);
    geoDirty_ = true;
    SetDirty();
}

Vector4f RSProperties::GetBounds() const
{
    return { boundsGeo_->GetX(), boundsGeo_->GetY(), boundsGeo_->GetWidth(), boundsGeo_->GetHeight() };
}

Vector2f RSProperties::GetBoundsSize() const
{
    return { boundsGeo_->GetWidth(), boundsGeo_->GetHeight() };
}

float RSProperties::GetBoundsWidth() const
{
    return boundsGeo_->GetWidth();
}

float RSProperties::GetBoundsHeight() const
{
    return boundsGeo_->GetHeight();
}

float RSProperties::GetBoundsPositionX() const
{
    return boundsGeo_->GetX();
}

float RSProperties::GetBoundsPositionY() const
{
    return boundsGeo_->GetY();
}

Vector2f RSProperties::GetBoundsPosition() const
{
    return { GetBoundsPositionX(), GetBoundsPositionY() };
}

void RSProperties::SetFrame(Vector4f frame)
{
    frameGeo_->SetRect(frame.x_, frame.y_, frame.z_, frame.w_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetFrameSize(Vector2f size)
{
    frameGeo_->SetSize(size.x_, size.y_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetFrameWidth(float width)
{
    frameGeo_->SetWidth(width);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetFrameHeight(float height)
{
    frameGeo_->SetHeight(height);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetFramePosition(Vector2f position)
{
    frameGeo_->SetPosition(position.x_, position.y_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetFramePositionX(float positionX)
{
    frameGeo_->SetX(positionX);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetFramePositionY(float positionY)
{
    frameGeo_->SetY(positionY);
    geoDirty_ = true;
    SetDirty();
}

Vector4f RSProperties::GetFrame() const
{
    return { frameGeo_->GetX(), frameGeo_->GetY(), frameGeo_->GetWidth(), frameGeo_->GetHeight() };
}

Vector2f RSProperties::GetFrameSize() const
{
    return { frameGeo_->GetWidth(), frameGeo_->GetHeight() };
}

float RSProperties::GetFrameWidth() const
{
    return frameGeo_->GetWidth();
}

float RSProperties::GetFrameHeight() const
{
    return frameGeo_->GetHeight();
}

float RSProperties::GetFramePositionX() const
{
    return frameGeo_->GetX();
}

float RSProperties::GetFramePositionY() const
{
    return frameGeo_->GetY();
}

Vector2f RSProperties::GetFramePosition() const
{
    return { GetFramePositionX(), GetFramePositionY() };
}

float RSProperties::GetFrameOffsetX() const
{
    return frameGeo_->IsEmpty() ? 0 : (frameGeo_->GetX() - boundsGeo_->GetX());
}

float RSProperties::GetFrameOffsetY() const
{
    return frameGeo_->IsEmpty() ? 0 : (frameGeo_->GetY() - boundsGeo_->GetY());
}

const std::shared_ptr<RSObjGeometry>& RSProperties::GetBoundsGeometry() const
{
    return boundsGeo_;
}

const std::shared_ptr<RSObjGeometry>& RSProperties::GetFrameGeometry() const
{
    return frameGeo_;
}

bool RSProperties::UpdateGeometry(const RSProperties* parent, bool dirtyFlag)
{
    if (boundsGeo_ == nullptr) {
        return false;
    }
    if (boundsGeo_->IsEmpty()) {
        boundsGeo_->SetPosition(frameGeo_->GetX(), frameGeo_->GetY());
        boundsGeo_->SetSize(frameGeo_->GetWidth(), frameGeo_->GetHeight());
        hasBounds_ = false;
    } else {
        hasBounds_ = true;
    }
#ifdef ROSEN_OHOS
    auto boundsGeoPtr = std::static_pointer_cast<RSObjAbsGeometry>(boundsGeo_);

    if (dirtyFlag || geoDirty_) {
        if (parent == nullptr) {
            boundsGeoPtr->UpdateMatrix(nullptr, 0.f, 0.f);
        } else {
            auto parentGeo = std::static_pointer_cast<RSObjAbsGeometry>(parent->boundsGeo_);
            boundsGeoPtr->UpdateMatrix(parentGeo, parent->GetFrameOffsetX(), parent->GetFrameOffsetY());
        }
    }
#endif
    return dirtyFlag || geoDirty_;
}

void RSProperties::SetPositionZ(float positionZ)
{
    boundsGeo_->SetZ(positionZ);
    frameGeo_->SetZ(positionZ);
    geoDirty_ = true;
    SetDirty();
}

float RSProperties::GetPositionZ() const
{
    return boundsGeo_->GetZ();
}

void RSProperties::SetPivot(Vector2f pivot)
{
    boundsGeo_->SetPivot(pivot.x_, pivot.y_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetPivotX(float pivotX)
{
    boundsGeo_->SetPivotX(pivotX);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetPivotY(float pivotY)
{
    boundsGeo_->SetPivotY(pivotY);
    geoDirty_ = true;
    SetDirty();
}

Vector2f RSProperties::GetPivot() const
{
    return { boundsGeo_->GetPivotX(), boundsGeo_->GetPivotY() };
}

float RSProperties::GetPivotX() const
{
    return boundsGeo_->GetPivotX();
}

float RSProperties::GetPivotY() const
{
    return boundsGeo_->GetPivotY();
}

void RSProperties::SetCornerRadius(float cornerRadius)
{
    if (!border_) {
        border_ = std::make_unique<Border>();
    }
    border_->cornerRadius_ = cornerRadius;
    SetDirty();
}

float RSProperties::GetCornerRadius() const
{
    return border_ ? border_->cornerRadius_ : 0.f;
}

void RSProperties::SetQuaternion(Quaternion quaternion)
{
    boundsGeo_->SetQuaternion(quaternion);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetRotation(float degree)
{
    boundsGeo_->SetRotation(degree);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetRotationX(float degree)
{
    boundsGeo_->SetRotationX(degree);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetRotationY(float degree)
{
    boundsGeo_->SetRotationY(degree);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetScale(Vector2f scale)
{
    boundsGeo_->SetScale(scale.x_, scale.y_);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetScaleX(float sx)
{
    boundsGeo_->SetScaleX(sx);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetScaleY(float sy)
{
    boundsGeo_->SetScaleY(sy);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetTranslate(Vector2f translate)
{
    boundsGeo_->SetTranslateX(translate[0]);
    boundsGeo_->SetTranslateY(translate[1]);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetTranslateX(float translate)
{
    boundsGeo_->SetTranslateX(translate);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetTranslateY(float translate)
{
    boundsGeo_->SetTranslateY(translate);
    geoDirty_ = true;
    SetDirty();
}

void RSProperties::SetTranslateZ(float translate)
{
    boundsGeo_->SetTranslateZ(translate);
    geoDirty_ = true;
    SetDirty();
}

Quaternion RSProperties::GetQuaternion() const
{
    return boundsGeo_->GetQuaternion();
}

float RSProperties::GetRotation() const
{
    return boundsGeo_->GetRotation();
}

float RSProperties::GetRotationX() const
{
    return boundsGeo_->GetRotationX();
}

float RSProperties::GetRotationY() const
{
    return boundsGeo_->GetRotationY();
}

float RSProperties::GetScaleX() const
{
    return boundsGeo_->GetScaleX();
}

float RSProperties::GetScaleY() const
{
    return boundsGeo_->GetScaleY();
}

Vector2f RSProperties::GetScale() const
{
    return { boundsGeo_->GetScaleX(), boundsGeo_->GetScaleY() };
}

Vector2f RSProperties::GetTranslate() const
{
    return Vector2f(GetTranslateX(), GetTranslateY());
}

float RSProperties::GetTranslateX() const
{
    return boundsGeo_->GetTranslateX();
}

float RSProperties::GetTranslateY() const
{
    return boundsGeo_->GetTranslateY();
}

float RSProperties::GetTranslateZ() const
{
    return boundsGeo_->GetTranslateZ();
}

void RSProperties::SetAlpha(float alpha)
{
    alpha_ = alpha;
    SetDirty();
}

float RSProperties::GetAlpha() const
{
    return alpha_;
}

void RSProperties::SetSublayerTransform(Matrix3f sublayerTransform)
{
    if (sublayerTransform_) {
        *sublayerTransform_ = sublayerTransform;
    } else {
        sublayerTransform_ = std::make_unique<Matrix3f>(sublayerTransform);
    }
    SetDirty();
}

Matrix3f RSProperties::GetSublayerTransform() const
{
    return (sublayerTransform_ != nullptr) ? *sublayerTransform_ : Matrix3f::IDENTITY;
}

// foreground properties
void RSProperties::SetForegroundColor(Color color)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->foregroundColor_ = color;
    SetDirty();
}

Color RSProperties::GetForegroundColor() const
{
    return decoration_ ? decoration_->foregroundColor_ : RgbPalette::Transparent();
}

// background properties
void RSProperties::SetBackgroundColor(Color color)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->backgroundColor_ = color;
    SetDirty();
}

Color RSProperties::GetBackgroundColor() const
{
    return decoration_ ? decoration_->backgroundColor_ : RgbPalette::Transparent();
}

void RSProperties::SetBackgroundShader(std::shared_ptr<RSShader> shader)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->bgShader_ = shader;
    SetDirty();
}

std::shared_ptr<RSShader> RSProperties::GetBackgroundShader() const
{
    return decoration_ ? decoration_->bgShader_ : nullptr;
}

void RSProperties::SetBgImage(std::shared_ptr<RSImage> image)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->bgImage_ = image;
    SetDirty();
}

std::shared_ptr<RSImage> RSProperties::GetBgImage() const
{
    return decoration_ ? decoration_->bgImage_ : nullptr;
}

void RSProperties::SetBgImageWidth(float width)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->bgImageRect_.width_ = width;
    SetDirty();
}

void RSProperties::SetBgImageHeight(float height)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->bgImageRect_.height_ = height;
    SetDirty();
}

void RSProperties::SetBgImagePositionX(float positionX)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->bgImageRect_.left_ = positionX;
    SetDirty();
}

void RSProperties::SetBgImagePositionY(float positionY)
{
    if (!decoration_) {
        decoration_ = std::make_unique<Decoration>();
    }
    decoration_->bgImageRect_.top_ = positionY;
    SetDirty();
}

float RSProperties::GetBgImageWidth() const
{
    return decoration_ ? decoration_->bgImageRect_.width_ : 0.f;
}

float RSProperties::GetBgImageHeight() const
{
    return decoration_ ? decoration_->bgImageRect_.height_ : 0.f;
}

float RSProperties::GetBgImagePositionX() const
{
    return decoration_ ? decoration_->bgImageRect_.left_ : 0.f;
}

float RSProperties::GetBgImagePositionY() const
{
    return decoration_ ? decoration_->bgImageRect_.top_ : 0.f;
}

// border properties
void RSProperties::SetBorderColor(Color color)
{
    if (!border_) {
        border_ = std::make_unique<Border>();
    }
    border_->borderColor_ = color;
    SetDirty();
}

void RSProperties::SetBorderWidth(float width)
{
    if (!border_) {
        border_ = std::make_unique<Border>();
    }
    border_->borderWidth_ = width;
    SetDirty();
}

void RSProperties::SetBorderStyle(BorderStyle style)
{
    if (!border_) {
        border_ = std::make_unique<Border>();
    }
    border_->borderStyle_ = style;
    SetDirty();
}

Color RSProperties::GetBorderColor() const
{
    return border_ ? border_->borderColor_ : RgbPalette::Transparent();
}

float RSProperties::GetBorderWidth() const
{
    return border_ ? border_->borderWidth_ : 0.f;
}

BorderStyle RSProperties::GetBorderStyle() const
{
    return border_ ? border_->borderStyle_ : BorderStyle::SOLID;
}

void RSProperties::SetBackgroundFilter(std::shared_ptr<RSFilter> backgroundFilter)
{
    backgroundFilter_ = backgroundFilter;
    SetDirty();
}

void RSProperties::SetFilter(std::shared_ptr<RSFilter> filter)
{
    filter_ = filter;
    SetDirty();
}

std::shared_ptr<RSFilter> RSProperties::GetBackgroundFilter() const
{
    return backgroundFilter_;
}

std::shared_ptr<RSFilter> RSProperties::GetFilter() const
{
    return filter_;
}

// shadow properties
void RSProperties::SetShadowColor(Color color)
{
    if (shadow_ == nullptr) {
        shadow_ = std::make_unique<RSShadow>();
    }
    shadow_->SetColor(color);
    SetDirty();
}

void RSProperties::SetShadowOffsetX(float offsetX)
{
    if (shadow_ == nullptr) {
        shadow_ = std::make_unique<RSShadow>();
    }
    shadow_->SetOffsetX(offsetX);
    SetDirty();
}

void RSProperties::SetShadowOffsetY(float offsetY)
{
    if (shadow_ == nullptr) {
        shadow_ = std::make_unique<RSShadow>();
    }
    shadow_->SetOffsetY(offsetY);
    SetDirty();
}

void RSProperties::SetShadowAlpha(float alpha)
{
    if (shadow_ == nullptr) {
        shadow_ = std::make_unique<RSShadow>();
    }
    shadow_->SetAlpha(alpha);
    SetDirty();
}

void RSProperties::SetShadowElevation(float elevation)
{
    if (shadow_ == nullptr) {
        shadow_ = std::make_unique<RSShadow>();
    }
    shadow_->SetElevation(elevation);
    SetDirty();
}

void RSProperties::SetShadowRadius(float radius)
{
    if (shadow_ == nullptr) {
        shadow_ = std::make_unique<RSShadow>();
    }
    shadow_->SetRadius(radius);
    SetDirty();
}

void RSProperties::SetShadowPath(std::shared_ptr<RSPath> shadowPath)
{
    if (shadow_ == nullptr) {
        shadow_ = std::make_unique<RSShadow>();
    }
    shadow_->SetPath(shadowPath);
    SetDirty();
}

Color RSProperties::GetShadowColor() const
{
    return shadow_ ? shadow_->GetColor() : Color(DEFAULT_SPOT_COLOR);
}

float RSProperties::GetShadowOffsetX() const
{
    return shadow_ ? shadow_->GetOffsetX() : DEFAULT_SHADOW_OFFSET_X;
}

float RSProperties::GetShadowOffsetY() const
{
    return shadow_ ? shadow_->GetOffsetY() : DEFAULT_SHADOW_OFFSET_Y;
}

float RSProperties::GetShadowAlpha() const
{
    return shadow_ ? shadow_->GetAlpha() : 0.f;
}

float RSProperties::GetShadowElevation() const
{
    return shadow_ ? shadow_->GetElevation() : 0.f;
}

float RSProperties::GetShadowRadius() const
{
    return shadow_ ? shadow_->GetRadius() : 0.f;
}

std::shared_ptr<RSPath> RSProperties::GetShadowPath() const
{
    return shadow_ ? shadow_->GetPath() : nullptr;
}

void RSProperties::SetFrameGravity(Gravity gravity)
{
    if (frameGravity_ != gravity) {
        frameGravity_ = gravity;
        SetDirty();
    }
}

Gravity RSProperties::GetFrameGravity() const
{
    return frameGravity_;
}

void RSProperties::SetClipBounds(std::shared_ptr<RSPath> path)
{
    if (clipPath_ != path) {
        clipPath_ = path;
        SetDirty();
    }
}

std::shared_ptr<RSPath> RSProperties::GetClipBounds() const
{
    return clipPath_;
}

void RSProperties::SetClipToBounds(bool clipToBounds)
{
    if (clipToBounds_ != clipToBounds) {
        clipToBounds_ = clipToBounds;
        SetDirty();
    }
}

bool RSProperties::GetClipToBounds() const
{
    return clipToBounds_;
}

void RSProperties::SetClipToFrame(bool clipToFrame)
{
    if (clipToFrame_ != clipToFrame) {
        clipToFrame_ = clipToFrame;
        SetDirty();
    }
}

bool RSProperties::GetClipToFrame() const
{
    return clipToFrame_;
}

RectF RSProperties::GetBoundsRect() const
{
    if (boundsGeo_->IsEmpty()) {
        return RectF(0, 0, GetFrameWidth(), GetFrameHeight());
    } else {
        return RectF(0, 0, GetBoundsWidth(), GetBoundsHeight());
    }
}

RectF RSProperties::GetFrameRect() const
{
    return RectF(0, 0, GetFrameWidth(), GetFrameHeight());
}

RectF RSProperties::GetBgImageRect() const
{
    return decoration_ ? decoration_->bgImageRect_ : RectF();
}

void RSProperties::SetVisible(bool visible)
{
    if (visible_ != visible) {
        visible_ = visible;
        SetDirty();
    }
}

bool RSProperties::GetVisible() const
{
    return visible_;
}

RRect RSProperties::GetRRect() const
{
    RectF rect = GetBoundsRect();
    RRect rrect = RRect(rect, GetCornerRadius(), GetCornerRadius());
    return rrect;
}

RRect RSProperties::GetInnerRRect() const
{
    auto rect = GetBoundsRect();
    float borderWidth = GetBorderWidth();
    rect.SetAll(rect.left_ + borderWidth, rect.top_ + borderWidth, rect.width_ - borderWidth * 2,
        rect.height_ - borderWidth * 2);
    float cornerRadius = std::max(0.0f, GetCornerRadius() - borderWidth);
    RRect rrect = RRect(rect, cornerRadius, cornerRadius);
    return rrect;
}

bool RSProperties::NeedFilter() const
{
    return (backgroundFilter_ != nullptr) || (filter_ != nullptr);
}

bool RSProperties::NeedClip() const
{
    return clipToBounds_ || clipToFrame_;
}

void RSProperties::SetDirty()
{
    isDirty_ = true;
}

void RSProperties::ResetDirty()
{
    isDirty_ = false;
    geoDirty_ = false;
}

bool RSProperties::IsDirty() const
{
    return isDirty_;
}

RectI RSProperties::GetDirtyRect() const
{
#ifdef ROSEN_OHOS
    auto boundsGeometry = std::static_pointer_cast<RSObjAbsGeometry>(boundsGeo_);
    if (clipToBounds_) {
        return boundsGeometry->GetAbsRect();
    } else {
        auto frameRect =
            boundsGeometry->MapAbsRect(RectF(GetFrameOffsetX(), GetFrameOffsetY(), GetFrameWidth(), GetFrameHeight()));
        return boundsGeometry->GetAbsRect().JoinRect(frameRect);
    }
#else
    return RectI();
#endif
}

void RSProperties::ResetBounds()
{
    if (!hasBounds_) {
        boundsGeo_->SetRect(0.f, 0.f, 0.f, 0.f);
    }
}

std::string RSProperties::Dump() const
{
    char buffer[UINT8_MAX];
    if (sprintf_s(buffer, UINT8_MAX, "Bounds[%.1f %.1f %.1f %.1f] Frame[%.1f %.1f %.1f %.1f] bgcolor[#%08X]",
        GetBoundsPositionX(), GetBoundsPositionY(), GetBoundsWidth(), GetBoundsHeight(),
        GetFramePositionX(), GetFramePositionY(), GetFrameWidth(), GetFrameHeight(),
        GetBackgroundColor().AsArgbInt()) != -1) {
        return std::string(buffer);
    }
    return "";
}
} // namespace Rosen
} // namespace OHOS
