/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include <memory>

#include "gtest/gtest.h"
#include "limit_number.h"

#include "pipeline/driven_render/rs_driven_render_ext.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS::Rosen {
class RSDrivenRenderExtTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void RSDrivenRenderExtTest::SetUpTestCase() {}
void RSDrivenRenderExtTest::TearDownTestCase() {}
void RSDrivenRenderExtTest::SetUp() {}
void RSDrivenRenderExtTest::TearDown() {}

/**
 * @tc.name: IsValidSurfaceName
 * @tc.desc: Test RSDrivenRenderExtTest.IsValidSurfaceName
 * @tc.type: FUNC
 * @tc.require: issueI6J4IL
 */
HWTEST_F(RSDrivenRenderExtTest, IsValidSurfaceName, TestSize.Level1)
{
    std::string surfaceName = "";
    bool isValideSurface = RSDrivenRenderExt::Instance().IsValidSurfaceName(surfaceName);
    ASSERT_EQ(false, isValideSurface);

    RSDrivenRenderExt::Instance().OpenDrivenRenderExt();
    isValideSurface = RSDrivenRenderExt::Instance().IsValidSurfaceName(surfaceName);
    ASSERT_EQ(false, isValideSurface);
}

/**
 * @tc.name: DisabledRenderMode
 * @tc.desc: Test RSDrivenRenderExtTest.DisabledRenderMode
 * @tc.type: FUNC
 * @tc.require: issueI6J4IL
 */
HWTEST_F(RSDrivenRenderExtTest, DisabledRenderMode, TestSize.Level1)
{
    DrivenExtInfo info;
    RSDrivenRenderExt::Instance().DisabledRenderMode(info);

    RSDrivenRenderExt::Instance().OpenDrivenRenderExt();
    RSDrivenRenderExt::Instance().DisabledRenderMode(info);
}

/**
 * @tc.name: SetCurrFrameBounds
 * @tc.desc: Test RSDrivenRenderExtTest.SetCurrFrameBounds
 * @tc.type: FUNC
 * @tc.require: issueI6J4IL
 */
HWTEST_F(RSDrivenRenderExtTest, SetCurrFrameBounds, TestSize.Level1)
{
    DrivenExtInfo info;
    RectF bounds;
    RectF viewPort;
    RectI contentAbsRect;
    RSDrivenRenderExt::Instance().SetCurrFrameBounds(info, bounds, viewPort, contentAbsRect);

    RSDrivenRenderExt::Instance().OpenDrivenRenderExt();
    RSDrivenRenderExt::Instance().SetCurrFrameBounds(info, bounds, viewPort, contentAbsRect);
}

/**
 * @tc.name: UpdateActivateFrameState
 * @tc.desc: Test RSDrivenRenderExtTest.UpdateActivateFrameState
 * @tc.type: FUNC
 * @tc.require: issueI6J4IL
 */
HWTEST_F(RSDrivenRenderExtTest, UpdateActivateFrameState, TestSize.Level1)
{
    DrivenExtInfo info;
    RectI dstRect;
    RSDrivenRenderExt::Instance().UpdateActivateFrameState(info, dstRect, false, false, false);

    RSDrivenRenderExt::Instance().OpenDrivenRenderExt();
    RSDrivenRenderExt::Instance().UpdateActivateFrameState(info, dstRect, false, false, false);
}

} // namespace OHOS::Rosen