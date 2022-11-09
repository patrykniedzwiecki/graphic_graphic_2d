/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, Hardware
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"
#include "limit_number.h"
#include "pipeline/rs_main_thread.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
class RSMainThreadTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void RSMainThreadTest::SetUpTestCase() {}
void RSMainThreadTest::TearDownTestCase() {}
void RSMainThreadTest::SetUp() {}
void RSMainThreadTest::TearDown() {}

/**
 * @tc.name: Start
 * @tc.desc: Test RSMainThreadTest.Start
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, Start, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->Start();
}

/**
 * @tc.name: RemoveRSEventDetector001
 * @tc.desc: Test RSMainThreadTest.RemoveRSEventDetector, with init
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, RemoveRSEventDetector001, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->InitRSEventDetector();
    mainThread->RemoveRSEventDetector();
}

/**
 * @tc.name: RemoveRSEventDetector002
 * @tc.desc: Test RSMainThreadTest.RemoveRSEventDetector, without init
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, RemoveRSEventDetector002, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->RemoveRSEventDetector();
}

/**
 * @tc.name: SetRSEventDetectorLoopStartTag001
 * @tc.desc: Test RSMainThreadTest.SetRSEventDetectorLoopStartTag, with init
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, SetRSEventDetectorLoopStartTag001, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->InitRSEventDetector();
    mainThread->SetRSEventDetectorLoopStartTag();
}

/**
 * @tc.name: SetRSEventDetectorLoopStartTag002
 * @tc.desc: Test RSMainThreadTest.SetRSEventDetectorLoopStartTag, without init
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, SetRSEventDetectorLoopStartTag002, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->SetRSEventDetectorLoopStartTag();
}

/**
 * @tc.name: SetRSEventDetectorLoopFinishTag001
 * @tc.desc: Test RSMainThreadTest.SetRSEventDetectorLoopFinishTag, with init
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, SetRSEventDetectorLoopFinishTag001, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->InitRSEventDetector();
    mainThread->SetRSEventDetectorLoopFinishTag();
}

/**
 * @tc.name: SetRSEventDetectorLoopFinishTag002
 * @tc.desc: Test RSMainThreadTest.SetRSEventDetectorLoopFinishTag, without init
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, SetRSEventDetectorLoopFinishTag002, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->SetRSEventDetectorLoopFinishTag();
}

/**
 * @tc.name: WaitUtilUniRenderFinished
 * @tc.desc: Test RSMainThreadTest.WaitUtilUniRenderFinished
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, WaitUtilUniRenderFinished, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->NotifyUniRenderFinish();
    mainThread->WaitUtilUniRenderFinished();
}

/**
 * @tc.name: ProcessCommandForDividedRender001
 * @tc.desc: Test RSMainThreadTest.ProcessCommandForDividedRender, waitingBufferAvailable_ is false
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, ProcessCommandForDividedRender001, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->waitingBufferAvailable_ = false;
    mainThread->ProcessCommandForDividedRender();
}

/**
 * @tc.name: ProcessCommandForDividedRender002
 * @tc.desc: Test RSMainThreadTest.ProcessCommandForDividedRender, followVisitorCommands_ is not empty
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, ProcessCommandForDividedRender002, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->followVisitorCommands_[0].emplace_back(nullptr);
    mainThread->ProcessCommandForDividedRender();
}

/**
 * @tc.name: CalcOcclusion
 * @tc.desc: Test RSMainThreadTest.CalcOcclusion, doWindowAnimate_ is false, useUniVisitor_ is true
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, CalcOcclusion, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->doWindowAnimate_ = false;
    mainThread->useUniVisitor_ = true;
    mainThread->CalcOcclusion();
}

/**
 * @tc.name: CheckQosVisChanged001
 * @tc.desc: Test RSMainThreadTest.CheckQosVisChanged, pidVisMap is empty
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, CheckQosVisChanged001, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    std::map<uint32_t, bool> pidVisMap;
    auto isVisibleChanged = mainThread->CheckQosVisChanged(pidVisMap);
    ASSERT_EQ(false, isVisibleChanged);
}

/**
 * @tc.name: CheckQosVisChanged002
 * @tc.desc: Test RSMainThreadTest.CheckQosVisChanged, pidVisMap is not empty
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, CheckQosVisChanged002, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    std::map<uint32_t, bool> pidVisMap;
    pidVisMap[0] = true;
    mainThread->lastPidVisMap_[0] = false;
    auto isVisibleChanged = mainThread->CheckQosVisChanged(pidVisMap);
    ASSERT_EQ(true, isVisibleChanged);
}

/**
 * @tc.name: CheckQosVisChanged003
 * @tc.desc: Test RSMainThreadTest.CheckQosVisChanged, pidVisMap is not empty, lastPidVisMap_ equals to pidVisMap
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, CheckQosVisChanged003, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    std::map<uint32_t, bool> pidVisMap;
    pidVisMap[0] = true;
    mainThread->lastPidVisMap_[0] = true;
    auto isVisibleChanged = mainThread->CheckQosVisChanged(pidVisMap);
    ASSERT_EQ(false, isVisibleChanged);
}

/**
 * @tc.name: Animate001
 * @tc.desc: Test RSMainThreadTest.Animate, doWindowAnimate_ is false
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, Animate001, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->doWindowAnimate_ = false;
    mainThread->Animate(0);
}

/**
 * @tc.name: Animate002
 * @tc.desc: Test RSMainThreadTest.Animate, doWindowAnimate_ is true
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, Animate002, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->doWindowAnimate_ = true;
    mainThread->Animate(0);
}

/**
 * @tc.name: UpdateRenderMode001
 * @tc.desc: Test RSMainThreadTest.UpdateRenderMode, waitingBufferAvailable_, waitingUpdateSurfaceNode_ is true
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, UpdateRenderMode001, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->waitingBufferAvailable_ = true;
    mainThread->waitingUpdateSurfaceNode_ = true;
    mainThread->UpdateRenderMode(false);
}

/**
 * @tc.name: UpdateRenderMode002
 * @tc.desc: Test RSMainThreadTest.UpdateRenderMode, waitingBufferAvailable_ is true,
 * waitingUpdateSurfaceNode_ is false
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, UpdateRenderMode002, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->waitingBufferAvailable_ = true;
    mainThread->waitingUpdateSurfaceNode_ = false;
    mainThread->UpdateRenderMode(false);
}

/**
 * @tc.name: UnRegisterOcclusionChangeCallback
 * @tc.desc: Test RSMainThreadTest.Animate, waitingBufferAvailable_, waitingUpdateSurfaceNode_ is true
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, UnRegisterOcclusionChangeCallback, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->UnRegisterOcclusionChangeCallback(nullptr);
}

/**
 * @tc.name: CleanOcclusionListener
 * @tc.desc: Test RSMainThreadTest.CleanOcclusionListener
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, CleanOcclusionListener, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    mainThread->CleanOcclusionListener();
}

/**
 * @tc.name: QosStateDump
 * @tc.desc: Test RSMainThreadTest.QosStateDump, str is an empty string
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, QosStateDump, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    std::string str = "";
    mainThread->QosStateDump(str);
}

/**
 * @tc.name: RenderServiceTreeDump
 * @tc.desc: Test RSMainThreadTest.RenderServiceTreeDump, str is an empty string
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, RenderServiceTreeDump, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    std::string str = "";
    mainThread->RenderServiceTreeDump(str);
}

/**
 * @tc.name: SetFocusAppInfo
 * @tc.desc: Test RSMainThreadTest.SetFocusAppInfo, input pid, uid is -1, str is an empty string
 * @tc.type:
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(RSMainThreadTest, SetFocusAppInfo, TestSize.Level1)
{
    auto mainThread = RSMainThread::Instance();
    std::string str = "";
    mainThread->SetFocusAppInfo(-1, -1, str, str);
}
} // namespace OHOS::Rosen
