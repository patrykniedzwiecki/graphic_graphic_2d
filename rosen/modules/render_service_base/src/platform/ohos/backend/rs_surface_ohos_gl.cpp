/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "rs_surface_ohos_gl.h"
#include "platform/common/rs_log.h"
#include "window.h"
#include <hilog/log.h>
#include "pipeline/rs_render_thread.h"

namespace OHOS {
namespace Rosen {

RSSurfaceOhosGl::RSSurfaceOhosGl(const sptr<Surface>& producer) : RSSurfaceOhos(producer)
{
    bufferUsage_ = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_MEM_DMA;
}

void RSSurfaceOhosGl::SetSurfaceBufferUsage(uint64_t usage)
{
    bufferUsage_ = usage;
}

RSSurfaceOhosGl::~RSSurfaceOhosGl()
{
    DestoryNativeWindow(mWindow);
    if (context_ != nullptr) {
        context_->DestroyEGLSurface(mEglSurface);
    }
    mWindow = nullptr;
    mEglSurface = EGL_NO_SURFACE;
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceOhosGl::RequestFrame(int32_t width, int32_t height, uint64_t uiTimestamp)
{
    RenderContext* context = GetRenderContext();
    if (context == nullptr) {
        ROSEN_LOGE("RSSurfaceOhosGl::RequestFrame, GetRenderContext failed!");
        return nullptr;
    }
    context->SetColorSpace(colorSpace_);
    if (mWindow == nullptr) {
        mWindow = CreateNativeWindowFromSurface(&producer_);
        mEglSurface = context->CreateEGLSurface((EGLNativeWindowType)mWindow);
        ROSEN_LOGD("RSSurfaceOhosGl: create and Init EglSurface %p", mEglSurface);
    }

    if (mEglSurface == EGL_NO_SURFACE) {
        ROSEN_LOGE("RSSurfaceOhosGl: Invalid eglSurface, return");
        return nullptr;
    }

    std::unique_ptr<RSSurfaceFrameOhosGl> frame = std::make_unique<RSSurfaceFrameOhosGl>(width, height);

    NativeWindowHandleOpt(mWindow, SET_USAGE, bufferUsage_);
    NativeWindowHandleOpt(mWindow, SET_BUFFER_GEOMETRY, width, height);
    NativeWindowHandleOpt(mWindow, GET_BUFFER_GEOMETRY, &mHeight, &mWidth);
    NativeWindowHandleOpt(mWindow, SET_COLOR_GAMUT, colorSpace_);
    NativeWindowHandleOpt(mWindow, SET_UI_TIMESTAMP, uiTimestamp);

    context->MakeCurrent(mEglSurface);

    ROSEN_LOGD("RSSurfaceOhosGl:RequestFrame, eglsurface is %p, width is %d, height is %d",
        mEglSurface, mWidth, mHeight);

    frame->SetRenderContext(context);

    std::unique_ptr<RSSurfaceFrame> ret(std::move(frame));

    return ret;
}

bool RSSurfaceOhosGl::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    RenderContext* context = GetRenderContext();
    if (context == nullptr) {
        ROSEN_LOGE("RSSurfaceOhosGl::FlushFrame, GetRenderContext failed!");
        return false;
    }

    // gpu render flush
    context->RenderFrame();
    context->SwapBuffers(mEglSurface);
    ROSEN_LOGD("RSSurfaceOhosGl: FlushFrame, SwapBuffers eglsurface is %p", mEglSurface);
    return true;
}

void RSSurfaceOhosGl::ClearBuffer()
{
    if (context_ != nullptr && mEglSurface != EGL_NO_SURFACE && producer_ != nullptr) {
        ROSEN_LOGD("RSSurfaceOhosGl: Clear surface buffer!");
        DestoryNativeWindow(mWindow);
        context_->MakeCurrent(EGL_NO_SURFACE);
        context_->DestroyEGLSurface(mEglSurface);
        mEglSurface = EGL_NO_SURFACE;
        mWindow = nullptr;
        producer_->GoBackground();
    }
}

void RSSurfaceOhosGl::ResetBufferAge()
{
    if (context_ != nullptr && mEglSurface != EGL_NO_SURFACE && producer_ != nullptr) {
        ROSEN_LOGD("RSSurfaceOhosGl: Reset Buffer Age!");
        DestoryNativeWindow(mWindow);
        context_->MakeCurrent(EGL_NO_SURFACE, context_->GetEGLContext());
        context_->DestroyEGLSurface(mEglSurface);
        mEglSurface = EGL_NO_SURFACE;
        mWindow = nullptr;
    }
}
} // namespace Rosen
} // namespace OHOS
