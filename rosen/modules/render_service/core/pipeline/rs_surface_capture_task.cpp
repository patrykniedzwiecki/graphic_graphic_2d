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

#include "pipeline/rs_surface_capture_task.h"
#include "pipeline/rs_main_thread.h"
#include "pipeline/rs_base_render_node.h"
#include "pipeline/rs_display_render_node.h"
#include "pipeline/rs_render_service_util.h"
#include "pipeline/rs_surface_render_node.h"
#include "platform/common/rs_log.h"
#include "platform/drawing/rs_surface.h"
#include "screen_manager/rs_screen_manager.h"
#include "screen_manager/rs_screen_mode_info.h"
#include <memory>

namespace OHOS {
namespace Rosen {
std::unique_ptr<Media::PixelMap> RSSurfaceCaptureTask::Run()
{
    if (scaleX_ <= 0 || scaleY_ <= 0) {
        ROSEN_LOGE("RSSurfaceCaptureTask::Run: SurfaceCapture scale is invalid.");
        return nullptr;
    }
    auto node = RSMainThread::Instance()->GetContext().GetNodeMap().GetRenderNode(nodeId_);
    if (node == nullptr) {
        ROSEN_LOGE("RSSurfaceCaptureTask::Run: node is nullptr");
        return nullptr;
    }
    std::unique_ptr<Media::PixelMap> pixelmap;
    std::shared_ptr<RSSurfaceCaptureVisitor> visitor = std::make_shared<RSSurfaceCaptureVisitor>();
    if (auto surfaceNode = node->ReinterpretCastTo<RSSurfaceRenderNode>()) {
        ROSEN_LOGI("RSSurfaceCaptureTask::Run: Into SURFACE_NODE SurfaceRenderNodeId:[%llu]", node->GetId());
        pixelmap = CreatePixelMapBySurfaceNode(surfaceNode);
        visitor->IsDisplayNode(false);
    } else if (auto displayNode = node->ReinterpretCastTo<RSDisplayRenderNode>()) {
        ROSEN_LOGI("RSSurfaceCaptureTask::Run: Into DISPLAY_NODE DisplayRenderNodeId:[%llu]", node->GetId());
        pixelmap = CreatePixelMapByDisplayNode(displayNode);
        visitor->IsDisplayNode(true);
    } else {
        ROSEN_LOGE("RSSurfaceCaptureTask::Run: Invalid RSRenderNodeType!");
        return nullptr;
    }
    if (pixelmap == nullptr) {
        ROSEN_LOGE("RSSurfaceCaptureTask::Run: pixelmap == nullptr!");
        return nullptr;
    }
    std::unique_ptr<SkCanvas> canvas = CreateCanvas(pixelmap);
    if (canvas == nullptr) {
        ROSEN_LOGE("RSSurfaceCaptureTask::Run: canvas is nullptr!");
        return nullptr;
    }
    visitor->SetCanvas(std::move(canvas));
    visitor->SetScale(scaleX_, scaleY_);
    node->Process(visitor);
    return pixelmap;
}

std::unique_ptr<Media::PixelMap> RSSurfaceCaptureTask::CreatePixelMapBySurfaceNode(
    std::shared_ptr<RSSurfaceRenderNode> node)
{
    if (node == nullptr) {
        ROSEN_LOGE("CreatePixelMapBySurfaceNode: node == nullptr");
        return nullptr;
    }
    int pixmapWidth = node->GetRenderProperties().GetBoundsWidth();
    int pixmapHeight = node->GetRenderProperties().GetBoundsHeight();
    Media::InitializationOptions opts;
    opts.size.width = ceil(pixmapWidth * scaleX_);
    opts.size.height = ceil(pixmapHeight * scaleY_);
    ROSEN_LOGD("RSSurfaceCaptureTask::CreatePixelMapBySurfaceNode: origin pixelmap width is [%u], height is [%u], "\
        "created pixelmap width is [%u], height is [%u], the scale is scaleY:[%f], scaleY:[%f]",
        pixmapWidth, pixmapHeight, opts.size.width, opts.size.height, scaleX_, scaleY_);
    return Media::PixelMap::Create(opts);
}

std::unique_ptr<Media::PixelMap> RSSurfaceCaptureTask::CreatePixelMapByDisplayNode(
    std::shared_ptr<RSDisplayRenderNode> node)
{
    if (node == nullptr) {
        ROSEN_LOGE("RSSurfaceCaptureTask::CreatePixelMapByDisplayNode: node is nullptr");
        return nullptr;
    }
    uint64_t screenId = node->GetScreenId();
    RSScreenModeInfo screenModeInfo;
    sptr<RSScreenManager> screenManager = CreateOrGetScreenManager();
    if (!screenManager) {
        ROSEN_LOGE("RSSurfaceCaptureTask::CreatePixelMapByDisplayNode: screenManager is nullptr!");
        return nullptr;
    }
    screenManager->GetScreenActiveMode(screenId, screenModeInfo);
    int pixmapWidth = screenModeInfo.GetScreenWidth();
    int pixmapHeight = screenModeInfo.GetScreenHeight();
    Media::InitializationOptions opts;
    opts.size.width = ceil(pixmapWidth * scaleX_);
    opts.size.height = ceil(pixmapHeight * scaleY_);
    ROSEN_LOGD("RSSurfaceCaptureTask::CreatePixelMapByDisplayNode: origin pixelmap width is [%u], height is [%u], "\
        "created pixelmap width is [%u], height is [%u], the scale is scaleY:[%f], scaleY:[%f]",
        pixmapWidth, pixmapHeight, opts.size.width, opts.size.height, scaleX_, scaleY_);
    return Media::PixelMap::Create(opts);
}

std::unique_ptr<SkCanvas> RSSurfaceCaptureTask::CreateCanvas(const std::unique_ptr<Media::PixelMap>& pixelmap)
{
    if (pixelmap == nullptr) {
        ROSEN_LOGE("RSSurfaceCaptureTask::CreateCanvas: pixelmap == nullptr");
        return nullptr;
    }
    auto address = const_cast<uint32_t*>(pixelmap->GetPixel32(0, 0));
    if (address == nullptr) {
        ROSEN_LOGE("RSSurfaceCaptureTask::CreateCanvas: address == nullptr");
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(pixelmap->GetWidth(), pixelmap->GetHeight(),
            kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    return SkCanvas::MakeRasterDirect(info, address, pixelmap->GetRowBytes());
}

void RSSurfaceCaptureTask::RSSurfaceCaptureVisitor::SetCanvas(std::unique_ptr<SkCanvas> canvas)
{
    if (canvas == nullptr) {
        ROSEN_LOGE("RSSurfaceCaptureTask::RSSurfaceCaptureVisitor::SetCanvas: address == nullptr");
        return;
    }
    canvas_ = std::move(canvas);
}

void RSSurfaceCaptureTask::RSSurfaceCaptureVisitor::ProcessDisplayRenderNode(RSDisplayRenderNode &node)
{
    ROSEN_LOGD("RsDebug RSSurfaceCaptureVisitor::ProcessDisplayRenderNode child size:[%d]", node.GetChildren().size());
    for (auto child : node.GetChildren()) {
        auto existingChild = child.lock();
        if (!existingChild) {
            ROSEN_LOGD("RSSurfaceCaptureVisitor::ProcessDisplayRenderNode this child haven't existed");
            continue;
        }
        existingChild->Process(shared_from_this());
    }
}

void RSSurfaceCaptureTask::RSSurfaceCaptureVisitor::ProcessSurfaceRenderNode(RSSurfaceRenderNode &node)
{
    if (node.GetBuffer() == nullptr) {
        ROSEN_LOGD("RSSurfaceCaptureTask::RSSurfaceCaptureVisitor::ProcessSurfaceRenderNode: node Buffer is nullptr!");
        return;
    }
    for (auto child : node.GetChildren()) {
        auto existingChild = child.lock();
        if (!existingChild) {
            ROSEN_LOGD("RSSurfaceCaptureVisitor::ProcessSurfaceRenderNode this child haven't existed");
            continue;
        }
        existingChild->Process(shared_from_this());
    }
    RsRenderServiceUtil::DrawBuffer(canvas_.get(), node.GetBuffer(), node, isDisplayNode_, scaleX_, scaleY_);
}
}
}