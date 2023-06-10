/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "ui/rs_ui_director.h"

#include <src/core/SkTraceEventCommon.h>

#include "rs_trace.h"
#include "sandbox_utils.h"

#include "command/rs_message_processor.h"
#include "modifier/rs_modifier_manager.h"
#include "modifier/rs_modifier_manager_map.h"
#include "pipeline/rs_frame_report.h"
#include "pipeline/rs_node_map.h"
#include "pipeline/rs_render_thread.h"
#include "platform/common/rs_log.h"
#include "platform/common/rs_system_properties.h"
#include "transaction/rs_application_agent_impl.h"
#include "transaction/rs_interfaces.h"
#include "transaction/rs_transaction_proxy.h"
#include "ui/rs_root_node.h"
#include "ui/rs_surface_extractor.h"
#include "ui/rs_surface_node.h"
#ifdef NEW_RENDER_CONTEXT
#include "memory/rs_memory_manager.h"
#endif

#ifdef _WIN32
#include <windows.h>
#define gettid GetCurrentThreadId
#endif

#ifdef __APPLE__
#define gettid getpid
#endif

#ifdef __gnu_linux__
#include <sys/types.h>
#include <sys/syscall.h>
#define gettid []() -> int32_t { return static_cast<int32_t>(syscall(SYS_gettid)); }
#endif

namespace OHOS {
namespace Rosen {
static TaskRunner g_uiTaskRunner;

std::shared_ptr<RSUIDirector> RSUIDirector::Create()
{
    return std::shared_ptr<RSUIDirector>(new RSUIDirector());
}

RSUIDirector::~RSUIDirector()
{
    Destroy();
}

void RSUIDirector::Init(bool shouldCreateRenderThread)
{
    AnimationCommandHelper::SetAnimationCallbackProcessor(AnimationCallbackProcessor);

    isUniRenderEnabled_ = RSSystemProperties::GetUniRenderEnabled();
    if (shouldCreateRenderThread && !isUniRenderEnabled_) {
        auto renderThreadClient = RSIRenderClient::CreateRenderThreadClient();
        auto transactionProxy = RSTransactionProxy::GetInstance();
        if (transactionProxy != nullptr) {
            transactionProxy->SetRenderThreadClient(renderThreadClient);
        }

        RsFrameReport::GetInstance().Init();
        if (!cacheDir_.empty()) {
            RSRenderThread::Instance().SetCacheDir(cacheDir_);
        }
        RSRenderThread::Instance().Start();
    }
    RSApplicationAgentImpl::Instance().RegisterRSApplicationAgent();

    GoForeground();

#ifdef SK_BUILD_TRACE_FOR_OHOS
    isSkiaTraceEnabled_ = RSSystemProperties::GetSkiaTraceEnabled();
    SkOHOSTraceUtil::setEnableTracing(isSkiaTraceEnabled_);
#endif
}

void RSUIDirector::GoForeground()
{
    ROSEN_LOGD("RSUIDirector::GoForeground");
    if (!isActive_) {
        if (!isUniRenderEnabled_) {
            RSRenderThread::Instance().UpdateWindowStatus(true);
        }
        isActive_ = true;
        if (auto node = RSNodeMap::Instance().GetNode<RSRootNode>(root_)) {
            node->SetEnableRender(true);
        }
        auto surfaceNode = surfaceNode_.lock();
        if (surfaceNode) {
            surfaceNode->MarkUIHidden(false);
        }
    }
}

void RSUIDirector::GoBackground()
{
    ROSEN_LOGD("RSUIDirector::GoBackground");
    if (isActive_) {
        if (!isUniRenderEnabled_) {
            RSRenderThread::Instance().UpdateWindowStatus(false);
        }
        isActive_ = false;
        if (auto node = RSNodeMap::Instance().GetNode<RSRootNode>(root_)) {
            node->SetEnableRender(false);
        }
        auto surfaceNode = surfaceNode_.lock();
        if (surfaceNode) {
            surfaceNode->MarkUIHidden(true);
        }
        // clean bufferQueue cache
        RSRenderThread::Instance().PostTask([surfaceNode]() {
            if (surfaceNode != nullptr) {
                std::shared_ptr<RSSurface> rsSurface = RSSurfaceExtractor::ExtractRSSurface(surfaceNode);
                rsSurface->ClearBuffer();
            }
        });
#ifdef ACE_ENABLE_GL
        RSRenderThread::Instance().PostTask([this]() {
            auto renderContext = RSRenderThread::Instance().GetRenderContext();
            if (renderContext != nullptr) {
#ifndef ROSEN_CROSS_PLATFORM
#if defined(NEW_RENDER_CONTEXT)
                MemoryManager::ClearRedundantResources(renderContext->GetGrContext());
#else
                renderContext->ClearRedundantResources();
#endif
#endif
            }
        });
#endif
    }
}

void RSUIDirector::Destroy()
{
    if (root_ != 0) {
        if (!isUniRenderEnabled_) {
            if (auto node = RSNodeMap::Instance().GetNode<RSRootNode>(root_)) {
                node->RemoveFromTree();
            }
        }
        root_ = 0;
    }
    GoBackground();
}

void RSUIDirector::SetRSSurfaceNode(std::shared_ptr<RSSurfaceNode> surfaceNode)
{
    surfaceNode_ = surfaceNode;
    AttachSurface();
}

void RSUIDirector::SetAbilityBGAlpha(uint8_t alpha)
{
    auto node = surfaceNode_.lock();
    if (!node) {
        ROSEN_LOGI("RSUIDirector::SetAbilityBGAlpha, surfaceNode_ is nullptr");
        return;
    }
    node->SetAbilityBGAlpha(alpha);
}

void RSUIDirector::SetRTRenderForced(bool isRenderForced)
{
    RSRenderThread::Instance().SetRTRenderForced(isRenderForced);
}

void RSUIDirector::SetContainerWindow(bool hasContainerWindow, float density)
{
    auto node = surfaceNode_.lock();
    if (!node) {
        ROSEN_LOGI("RSUIDirector::SetContainerWindow, surfaceNode_ is nullptr");
        return;
    }
    node->SetContainerWindow(hasContainerWindow, density);
}

void RSUIDirector::SetRoot(NodeId root)
{
    if (root_ == root) {
        ROSEN_LOGW("RSUIDirector::SetRoot, root_ is not change");
        return;
    }
    root_ = root;
    AttachSurface();
}

void RSUIDirector::AttachSurface()
{
    auto node = RSNodeMap::Instance().GetNode<RSRootNode>(root_);
    auto surfaceNode = surfaceNode_.lock();
    if (node != nullptr && surfaceNode != nullptr) {
        node->AttachRSSurfaceNode(surfaceNode);
        ROSEN_LOGD("RSUIDirector::AttachSurface [%" PRIu64 "]", surfaceNode->GetId());
    } else {
        ROSEN_LOGD("RSUIDirector::AttachSurface not ready");
    }
}

void RSUIDirector::SetAppFreeze(bool isAppFreeze)
{
    auto surfaceNode = surfaceNode_.lock();
    if (surfaceNode != nullptr) {
        surfaceNode->SetFreeze(isAppFreeze);
    }
}

void RSUIDirector::SetTimeStamp(uint64_t timeStamp, const std::string& abilityName)
{
    timeStamp_ = timeStamp;
    abilityName_ = abilityName;
}

void RSUIDirector::SetCacheDir(const std::string& cacheFilePath)
{
    cacheDir_ = cacheFilePath;
}

bool RSUIDirector::RunningCustomAnimation(uint64_t timeStamp)
{
    bool hasRunningAnimation = false;
    auto modifierManager = RSModifierManagerMap::Instance()->GetModifierManager(gettid());
    if (modifierManager == nullptr) {
        return hasRunningAnimation;
    }

    hasRunningAnimation = modifierManager->Animate(timeStamp);
    modifierManager->Draw();

    // post animation finish callback(s) to task queue
    RSUIDirector::RecvMessages();
    return hasRunningAnimation;
}

void RSUIDirector::SetUITaskRunner(const TaskRunner& uiTaskRunner)
{
    g_uiTaskRunner = uiTaskRunner;
}

void RSUIDirector::SendMessages()
{
    ROSEN_TRACE_BEGIN(HITRACE_TAG_GRAPHIC_AGP, "SendCommands");
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy != nullptr) {
        transactionProxy->FlushImplicitTransaction(timeStamp_, abilityName_);
    }
    ROSEN_TRACE_END(HITRACE_TAG_GRAPHIC_AGP);
}

void RSUIDirector::RecvMessages()
{
    if (GetRealPid() == -1) {
        return;
    }
    static const uint32_t pid = static_cast<uint32_t>(GetRealPid());
    if (!RSMessageProcessor::Instance().HasTransaction(pid)) {
        return;
    }
    static std::mutex recvMessagesMutex;
    std::unique_lock<std::mutex> lock(recvMessagesMutex);
    auto transactionDataPtr = std::make_shared<RSTransactionData>(RSMessageProcessor::Instance().GetTransaction(pid));
    RecvMessages(transactionDataPtr);
}

void RSUIDirector::RecvMessages(std::shared_ptr<RSTransactionData> cmds)
{
    if (cmds == nullptr || cmds->IsEmpty()) {
        return;
    }
    ROSEN_LOGD("RSUIDirector::RecvMessages success");
    PostTask([cmds]() {
        ROSEN_LOGD("RSUIDirector::ProcessMessages success");
        RSUIDirector::ProcessMessages(cmds);
    });
}

void RSUIDirector::ProcessMessages(std::shared_ptr<RSTransactionData> cmds)
{
    static RSContext context; // RSCommand->process() needs it
    cmds->Process(context);
}

void RSUIDirector::AnimationCallbackProcessor(NodeId nodeId, AnimationId animId, AnimationCallbackEvent event)
{
    // try find the node by nodeId
    if (auto nodePtr = RSNodeMap::Instance().GetNode<RSNode>(nodeId)) {
        if (!nodePtr->AnimationCallback(animId, event)) {
            ROSEN_LOGE("RSUIDirector::AnimationCallbackProcessor, could not find animation %" PRIu64 " on node %" PRIu64
                       ".", animId, nodeId);
        }
        return;
    }

    // if node not found, try fallback node
    auto& fallbackNode = RSNodeMap::Instance().GetAnimationFallbackNode();
    if (fallbackNode && fallbackNode->AnimationCallback(animId, event)) {
        ROSEN_LOGD("RSUIDirector::AnimationCallbackProcessor, found animation %" PRIu64 " on fallback node.", animId);
    } else {
        ROSEN_LOGE("RSUIDirector::AnimationCallbackProcessor, could not find animation %" PRIu64 " on fallback node.",
            animId);
    }
}

void RSUIDirector::PostTask(const std::function<void()>& task)
{
    if (g_uiTaskRunner == nullptr) {
        ROSEN_LOGE("RSUIDirector::PostTask, uiTaskRunner is null");
        return;
    }
    ROSEN_LOGD("RSUIDirector::PostTask success");
    g_uiTaskRunner(task);
}
} // namespace Rosen
} // namespace OHOS
