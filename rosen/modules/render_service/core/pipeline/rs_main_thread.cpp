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
#include "pipeline/rs_main_thread.h"
#include <memory>
#include <string>
#include <securec.h>

#include "command/rs_message_processor.h"
#include "pipeline/rs_base_render_node.h"
#include "pipeline/rs_base_render_util.h"
#include "pipeline/rs_divided_render_util.h"
#include "pipeline/rs_render_service_visitor.h"
#include "pipeline/rs_surface_render_node.h"
#include "pipeline/rs_uni_render_judgement.h"
#include "pipeline/rs_uni_render_visitor.h"
#include "platform/common/rs_log.h"
#include "platform/drawing/rs_vsync_client.h"
#include "rs_trace.h"
#include "screen_manager/rs_screen_manager.h"
#include "transaction/rs_transaction_proxy.h"

namespace OHOS {
namespace Rosen {
RSMainThread* RSMainThread::Instance()
{
    static RSMainThread instance;
    return &instance;
}

RSMainThread::RSMainThread() : mainThreadId_(std::this_thread::get_id())
{
}

RSMainThread::~RSMainThread() noexcept
{
}

void RSMainThread::Init()
{
    mainLoop_ = [&]() {
        RS_LOGD("RsDebug mainLoop start");
        ROSEN_TRACE_BEGIN(HITRACE_TAG_GRAPHIC_AGP, "RSMainThread::DoComposition");
        ConsumeAndUpdateAllNodes();
        ProcessCommand();
        Animate(timestamp_);
        Render();
        ReleaseAllNodesBuffer();
        SendCommands();
        ROSEN_TRACE_END(HITRACE_TAG_GRAPHIC_AGP);
        RS_LOGD("RsDebug mainLoop end");
    };

    runner_ = AppExecFwk::EventRunner::Create(false);
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);

    sptr<VSyncConnection> conn = new VSyncConnection(rsVSyncDistributor_, "rs");
    rsVSyncDistributor_->AddConnection(conn);
    receiver_ = std::make_shared<VSyncReceiver>(conn);
    receiver_->Init();
    RSDividedRenderUtil::InitEnableClient();

    renderEngine_ = std::make_shared<RSRenderEngine>();
}

void RSMainThread::Start()
{
    if (runner_) {
        runner_->Run();
    }
}

void RSMainThread::ProcessCommand()
{
    const auto& nodeMap = context_.GetNodeMap();
    RS_TRACE_BEGIN("RSMainThread::ProcessCommand");
    {
        std::lock_guard<std::mutex> lock(transitionDataMutex_);
        if (cacheCommand_.find(0) != cacheCommand_.end()) {
            effectCommand_.insert(effectCommand_.end(),
                std::make_move_iterator(cacheCommand_[0][0].begin()), std::make_move_iterator(cacheCommand_[0][0].end()));
            cacheCommand_[0].clear();
        }
        for (auto it = cacheCommand_.begin(); it != cacheCommand_.end();) {
            auto surfaceNodeId = it->first;
            if (surfaceNodeId == 0) {
                it++;
                continue;
            }
            auto node = nodeMap.GetRenderNode<RSSurfaceRenderNode>(surfaceNodeId);
            std::map<uint64_t, std::vector<std::unique_ptr<RSCommand>>>::iterator effectIter;
            std::unordered_map<NodeId, uint64_t>::iterator bufferTimestamp = bufferTimestamps_.find(surfaceNodeId);

            if (!node || !node->IsOnTheTree() || bufferTimestamp == bufferTimestamps_.end()) {
                // If node has been destructed or is not on the tree or has no valid buffer,
                // for all command cached in cacheCommand_[surfaceNodeId] should be executed immediately
                effectIter = cacheCommand_[surfaceNodeId].end();
            } else {
                uint64_t timestamp = bufferTimestamp->second;
                effectIter = cacheCommand_[surfaceNodeId].upper_bound(timestamp);
            }

            for (auto it2 = cacheCommand_[surfaceNodeId].begin(); it2 != effectIter; it2++) {
                effectCommand_.insert(effectCommand_.end(),
                    std::make_move_iterator(it2->second.begin()), std::make_move_iterator(it2->second.end()));
            }
            cacheCommand_[surfaceNodeId].erase(cacheCommand_[surfaceNodeId].begin(), effectIter);

            for (auto it2 = cacheCommand_[surfaceNodeId].begin(); it2 != cacheCommand_[surfaceNodeId].end(); it2++) {
                RS_LOGD("RSMainThread::ProcessCommand CacheCommand NodeId = %llu, timestamp = %llu, commandSize = %zu",
                    surfaceNodeId, it2->first, it2->second.size());
            }

            it++;
        }
    }
    for (size_t i = 0; i < effectCommand_.size(); i++) {
        auto rsCommand = std::move(effectCommand_[i]);
        if (rsCommand) {
            rsCommand->Process(context_);
        }
    }
    effectCommand_.clear();
    RS_TRACE_END();
}

void RSMainThread::ConsumeAndUpdateAllNodes()
{
    bool needRequestNextVsync = false;

    bufferTimestamps_.clear();
    const auto& nodeMap = GetContext().GetNodeMap();
    nodeMap.TraversalNodes([this, &needRequestNextVsync](const std::shared_ptr<RSBaseRenderNode>& node) mutable {
        if (node == nullptr) {
            return;
        }

        if (node->IsInstanceOf<RSSurfaceRenderNode>()) {
            RSSurfaceRenderNode& surfaceNode = *(RSBaseRenderNode::ReinterpretCast<RSSurfaceRenderNode>(node));
            RSSurfaceHandler& surfaceHandler = static_cast<RSSurfaceHandler&>(surfaceNode);
            if (RSBaseRenderUtil::ConsumeAndUpdateBuffer(surfaceHandler)) {
                this->bufferTimestamps_[surfaceNode.GetId()] = static_cast<uint64_t>(surfaceNode.GetTimestamp());
            }

            // still have buffer(s) to consume.
            if (surfaceHandler.GetAvailableBufferCount() > 0) {
                needRequestNextVsync = true;
            }
        }
    });

    if (needRequestNextVsync) {
        RequestNextVSync();
    }
}

void RSMainThread::ReleaseAllNodesBuffer()
{
    const auto& nodeMap = GetContext().GetNodeMap();
    nodeMap.TraversalNodes([this](const std::shared_ptr<RSBaseRenderNode>& node) mutable {
        if (node == nullptr) {
            return;
        }

        if (node->IsInstanceOf<RSSurfaceRenderNode>()) {
            RSSurfaceRenderNode& surfaceNode = *(RSBaseRenderNode::ReinterpretCast<RSSurfaceRenderNode>(node));
            RSSurfaceHandler& surfaceHandler = static_cast<RSSurfaceHandler&>(surfaceNode);
            (void)RSBaseRenderUtil::ReleaseBuffer(surfaceHandler);
        }
    });
}

void RSMainThread::WaitUtilUniRenderFinished()
{
    std::unique_lock<std::mutex> lock(uniRenderMutex_);
    if (uniRenderFinished_) {
        return;
    }
    uniRenderCond_.wait(lock, [this]() { return uniRenderFinished_; });
    uniRenderFinished_ = false;
}

void RSMainThread::NotifyUniRenderFinish()
{
    if (std::this_thread::get_id() != Id()) {
        std::lock_guard<std::mutex> lock(uniRenderMutex_);
        uniRenderFinished_ = true;
        uniRenderCond_.notify_one();
    } else {
        uniRenderFinished_ = true;
    }
}

void RSMainThread::Render()
{
    const std::shared_ptr<RSBaseRenderNode> rootNode = context_.GetGlobalRootRenderNode();
    if (rootNode == nullptr) {
        RS_LOGE("RSMainThread::Draw GetGlobalRootRenderNode fail");
        return;
    }
    std::shared_ptr<RSNodeVisitor> visitor;
    if (RSUniRenderJudgement::IsUniRender()) {
        RS_LOGI("RSMainThread::Render isUni");
        visitor = std::make_shared<RSUniRenderVisitor>();
    } else {
        auto rsVisitor = std::make_shared<RSRenderServiceVisitor>();
        rsVisitor->SetAnimateState(doAnimate_);
        visitor = rsVisitor;
    }
    rootNode->Prepare(visitor);
    CalcOcclusion();
    rootNode->Process(visitor);

    renderEngine_->ShrinkEGLImageCachesIfNeeded();
}

void RSMainThread::CalcOcclusion()
{
    if (doAnimate_) {
        return;
    }
    const std::shared_ptr<RSBaseRenderNode> node = context_.GetGlobalRootRenderNode();
    if (node == nullptr) {
        RS_LOGE("RSMainThread::CalcOcclusion GetGlobalRootRenderNode fail");
        return;
    }

    std::vector<RSBaseRenderNode::SharedPtr> curAllSurfaces;
    node->CollectSurface(node, curAllSurfaces);
    // 1. Judge whether it is dirty
    // Surface cnt changed or surface DstRectChanged or surface ZorderChanged
    bool winDirty = lastSurafceCnt_ != curAllSurfaces.size();
    lastSurafceCnt_ = curAllSurfaces.size();
    if (!winDirty) {
        for (auto it = curAllSurfaces.rbegin(); it != curAllSurfaces.rend(); ++it) {
            auto surface = RSBaseRenderNode::ReinterpretCast<RSSurfaceRenderNode>(*it);
            if (surface == nullptr) {
                continue;
            }
            if (surface->GetRenderProperties().GetZorderChanged() || surface->GetDstRectChanged()) {
                winDirty = true;
            }
            surface->GetMutableRenderProperties().CleanZorderChanged();
            surface->CleanDstRectChanged();
        }
    }
    if (!winDirty) {
        return;
    }

    RS_TRACE_BEGIN("RSMainThread::CalcOcclusion");
    // 2. Calc occlusion
    Occlusion::Region curRegion;
    VisibleData curVisVec;
    for (auto it = curAllSurfaces.rbegin(); it != curAllSurfaces.rend(); ++it) {
        auto surface = RSBaseRenderNode::ReinterpretCast<RSSurfaceRenderNode>(*it);
        if (surface == nullptr || surface->GetDstRect().IsEmpty() ||
            surface->GetRenderProperties().GetAlpha() < 1.0) {
            continue;
        }
        Occlusion::Rect rect { surface->GetDstRect().left_, surface->GetDstRect().top_,
                               surface->GetDstRect().GetRight(), surface->GetDstRect().GetBottom() };
        Occlusion::Region curSurface { rect };
        // Current surface subtract current region, if result region is empty that means it's covered
        Occlusion::Region subResult = curSurface.Sub(curRegion);
        std::string info = "RSMainThread::CalcOcclusion: id: " + std::to_string(surface->GetId()) + ", ";
        info.append("name: " + surface->GetName()) + ", ";
        info.append(subResult.GetRegionInfo());
        RS_LOGD(info.c_str());
        // Set relult to SurfaceRenderNode and its children
        surface->SetVisibleRegionRecursive(subResult, curVisVec);
        // Current region need to merge current surface for next calculation
        curRegion = curSurface.Or(curRegion);
    }

    // 3. Callback to WMS
    CallbackToWMS(curVisVec);
    RS_TRACE_END();
}

void RSMainThread::CallbackToWMS(VisibleData& curVisVec)
{
    // if visible surfaces changed callback to WMS：
    // 1. curVisVec size changed
    // 2. curVisVec content changed
    bool visibleChanged = curVisVec.size() != lastVisVec_.size();
    std::sort(curVisVec.begin(), curVisVec.end());
    if (!visibleChanged) {
        for (uint32_t i = 0; i < curVisVec.size(); i++) {
            if (curVisVec[i] != lastVisVec_[i]) {
                visibleChanged = true;
                break;
            }
        }
    }
    if (visibleChanged) {
        std::string inf;
        char strBuffer[UINT8_MAX] = { 0 };
        if (sprintf_s(strBuffer, UINT8_MAX, "RSMainThread::CallbackToWMS:%d - %d",
            lastVisVec_.size(), curVisVec.size()) != -1) {
            inf.append(strBuffer);
        }
        RS_TRACE_NAME(inf.c_str());
        for (auto& listener : occlusionListeners_) {
            listener->OnOcclusionVisibleChanged(std::make_shared<RSOcclusionData>(curVisVec));
        }
    }
    lastVisVec_.clear();
    std::copy(curVisVec.begin(), curVisVec.end(), std::back_inserter(lastVisVec_));
}

void RSMainThread::RequestNextVSync()
{
    RS_TRACE_FUNC();
    VSyncReceiver::FrameCallback fcb = {
        .userData_ = this,
        .callback_ = std::bind(&RSMainThread::OnVsync, this, ::std::placeholders::_1, ::std::placeholders::_2),
    };
    if (receiver_ != nullptr) {
        receiver_->RequestNextVSync(fcb);
    }
}

void RSMainThread::OnVsync(uint64_t timestamp, void *data)
{
    ROSEN_TRACE_BEGIN(HITRACE_TAG_GRAPHIC_AGP, "RSMainThread::OnVsync");
    timestamp_ = timestamp;
    if (handler_) {
        handler_->PostTask(mainLoop_, AppExecFwk::EventQueue::Priority::IDLE);

        auto screenManager_ = CreateOrGetScreenManager();
        if (screenManager_ != nullptr) {
            PostTask([=](){
                screenManager_->ProcessScreenHotPlugEvents();
            });
        }
    }
    ROSEN_TRACE_END(HITRACE_TAG_GRAPHIC_AGP);
}

void RSMainThread::Animate(uint64_t timestamp)
{
    RS_TRACE_FUNC();

    if (context_.animatingNodeList_.empty()) {
        doAnimate_ = false;
        return;
    }

    RS_LOGD("RSMainThread::Animate start, processing %d animating nodes", context_.animatingNodeList_.size());

    doAnimate_ = true;
    // iterate and animate all animating nodes, remove if animation finished
    std::__libcpp_erase_if_container(context_.animatingNodeList_, [timestamp](const auto& iter) -> bool {
        auto node = iter.second.lock();
        if (node == nullptr) {
            RS_LOGD("RSMainThread::Animate removing expired animating node");
            return true;
        }
        bool animationFinished = !node->Animate(timestamp);
        if (animationFinished) {
            RS_LOGD("RSMainThread::Animate removing finished animating node %llu", node->GetId());
        }
        return animationFinished;
    });

    RS_LOGD("RSMainThread::Animate end, %d animating nodes remains", context_.animatingNodeList_.size());

    RequestNextVSync();
}

void RSMainThread::RecvRSTransactionData(std::unique_ptr<RSTransactionData>& rsTransactionData)
{
    const auto& nodeMap = context_.GetNodeMap();
    {
        std::lock_guard<std::mutex> lock(transitionDataMutex_);
        std::unique_ptr<RSTransactionData> transactionData(std::move(rsTransactionData));
        if (transactionData) {
            auto nodeIds = transactionData->GetNodeIds();
            auto followTypes = transactionData->GetFollowTypes();
            auto& commands = transactionData->GetCommands();
            auto timestamp = transactionData->GetTimestamp();
            RS_LOGD("RSMainThread::RecvRSTransactionData timestamp = %llu", timestamp);
            for (int i = 0; i < transactionData->GetCommandCount(); i++) {
                auto nodeId = nodeIds[i];
                auto followtype = followTypes[i];
                std::unique_ptr<RSCommand> command = std::move(commands[i]);
                if (nodeId == 0 || followtype == FollowType::NONE) {
                    cacheCommand_[0][0].emplace_back(std::move(command));
                } else {
                    auto node = nodeMap.GetRenderNode<RSBaseRenderNode>(nodeId);
                    if (node && followtype == FollowType::FOLLOW_TO_PARENT) {
                        auto parentNode = node->GetParent().lock();
                        if (parentNode) {
                            nodeId = parentNode->GetId();
                        } else {
                            cacheCommand_[0][0].emplace_back(std::move(command));
                            continue;
                        }
                    }
                    cacheCommand_[nodeId][timestamp].emplace_back(std::move(command));
                }
            }
        }
    }
    RequestNextVSync();
}

void RSMainThread::PostTask(RSTaskMessage::RSTask task)
{
    if (handler_) {
        handler_->PostTask(task, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}

void RSMainThread::RegisterApplicationAgent(uint32_t pid, sptr<IApplicationAgent> app)
{
    applicationAgentMap_.emplace(pid, app);
}

void RSMainThread::UnRegisterApplicationAgent(sptr<IApplicationAgent> app)
{
    std::__libcpp_erase_if_container(applicationAgentMap_, [&app](auto& iter) { return iter.second == app; });
}

void RSMainThread::RegisterOcclusionChangeCallback(sptr<RSIOcclusionChangeCallback> callback)
{
    occlusionListeners_.emplace_back(callback);
}

void RSMainThread::UnRegisterOcclusionChangeCallback(sptr<RSIOcclusionChangeCallback> callback)
{
    auto iter = std::find(occlusionListeners_.begin(), occlusionListeners_.end(), callback);
    if (iter != occlusionListeners_.end()) {
        occlusionListeners_.erase(iter);
    }
}

void RSMainThread::CleanOcclusionListener()
{
    occlusionListeners_.clear();
}

void RSMainThread::SendCommands()
{
    RS_TRACE_FUNC();
    if (!RSMessageProcessor::Instance().HasTransaction()) {
        return;
    }

    // dispatch messages to corresponding application
    auto transactionMapPtr = std::make_shared<std::unordered_map<uint32_t, RSTransactionData>>(
        RSMessageProcessor::Instance().GetAllTransactions());
    PostTask([this, transactionMapPtr]() {
        for (auto& transactionIter : *transactionMapPtr) {
            auto pid = transactionIter.first;
            auto appIter = applicationAgentMap_.find(pid);
            if (appIter == applicationAgentMap_.end()) {
                RS_LOGI("RSMainThread::SendCommand no application found for pid %d", pid);
                continue;
            }
            auto& app = appIter->second;
            auto transactionPtr = std::make_shared<RSTransactionData>(std::move(transactionIter.second));
            app->OnTransaction(transactionPtr);
        }
    });
}

void RSMainThread::RenderServiceTreeDump(std::string& dumpString)
{
    dumpString.append("\n");
    dumpString.append("-- RenderServiceTreeDump: \n");
    const std::shared_ptr<RSBaseRenderNode> rootNode = context_.GetGlobalRootRenderNode();
    if (rootNode == nullptr) {
        dumpString.append("rootNode is null\n");
        return;
    }
    rootNode->DumpTree(0, dumpString);
}
} // namespace Rosen
} // namespace OHOS
