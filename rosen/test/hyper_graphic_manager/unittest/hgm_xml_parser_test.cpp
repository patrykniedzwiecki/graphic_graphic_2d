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

#include <gtest/gtest.h>
#include <test_header.h>

#include "xml_parser.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class HgmXmlParserTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HyperGraphicManagerTest::SetUpTestCase() {}
void HyperGraphicManagerTest::TearDownTestCase() {}
void HyperGraphicManagerTest::SetUp() {}
void HyperGraphicManagerTest::SetUp() {}

/**
 * @tc.name: LoadConfiguration
 * @tc.desc: Verify the result of LoadConfiguration function
 * @tc.type: FUNC
 * @tc.require: I7DMS1
 */
HWTEST_F(HgmXmlParserTest, LoadConfiguration, Function | SmallTest | Level1)
{
    std::unique_ptr<XMLParser> parser = std::make_unique<XMLParser>();

    PART("CaseDescription") {
        STEP("1. get an xml parser") {
            STEP_ASSERT_NE(parser, nullptr);
        }
        STEP("2. check the result of configuration") {
            int32_t load = parser->LoadConfiguration();
            STEP_ASSERT_EQ(load, 0);
        }
    }
}

/**
 * @tc.name: Parse
 * @tc.desc: Verify the result of parsing functions
 * @tc.type: FUNC
 * @tc.require: I7DMS1
 */
HWTEST_F(HgmXmlParserTest, Parse, Function | SmallTest | Level1)
{
    std::unique_ptr<XMLParser> parser = std::make_unique<XMLParser>();
    int32_t load = parser->LoadConfiguration();
    int32_t parse = parser->Parse();
    auto parsedData = parser->GetParsedData();

    PART("CaseDescription") {
        STEP("1. get an xml parser") {
            STEP_ASSERT_EQ(load, 0);
            STEP_ASSERT_EQ(parse, 0);
        }
        STEP("2. check the parsing result ") {
            STEP_ASSERT_EQ(parsedData->isDynamicFrameRateEnable_, "1");
            STEP_ASSERT_NE(parsedData->customerSettingConfig_.size(), 0);
            STEP_ASSERT_NE(parsedData->detailedStrategies_.size(), 0);
            STEP_ASSERT_NE(parsedData->animationDynamicStrats_, 0);
        }
    }
}
} // namespace Rosen
} // namespace OHOS