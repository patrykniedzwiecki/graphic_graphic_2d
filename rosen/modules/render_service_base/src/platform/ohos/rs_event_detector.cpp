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

#include "sandbox_utils.h"
#include "platform/common/rs_event_manager.h"

#ifdef RES_SCHED_ENABLE
#include "res_sched_client.h"
#endif

namespace OHOS {
namespace Rosen {
namespace {
#ifdef RES_SCHED_ENABLE
constexpr uint32_t FRAME_RATE_NUM           = 60;
constexpr uint32_t RES_TYPE_CLICK_ANIMATION = 35;
constexpr int32_t CLICK_ANIMATION_NORMAL    = 1;
constexpr int32_t CLICK_ANIMATION_SOON      = 2;
constexpr int32_t CLICK_ANIMATION_BOOST     = 3;
constexpr int32_t THRESHOLD_PARA_2          = 2;
constexpr int32_t THRESHOLD_PARA_3          = 3;
#endif
}
uint64_t RSEventTimer::GetSysTimeMs()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

std::shared_ptr<RSBaseEventDetector> RSBaseEventDetector::CreateRSTimeOutDetector(int timeOutThresholdMs,
    std::string detectorStringId)
{
    std::shared_ptr<RSTimeOutDetector> eventPtr = std::make_shared<RSTimeOutDetector>(timeOutThresholdMs,
    detectorStringId);
    return  eventPtr;
}

RSTimeOutDetector::RSTimeOutDetector(int timeOutThresholdMs,
    std::string detectorStringId) :RSBaseEventDetector(detectorStringId)
{
    RS_LOGD("RSTimeOutDetector ::RSTimeOutDetector timeOutThresholdMs is %d ", timeOutThresholdMs);
    timeOutThresholdMs_ = timeOutThresholdMs;
    paramList_["timeOutThresholdMs"] = std::to_string(timeOutThresholdMs_);
}

void RSTimeOutDetector::SetParam(const std::string& key, const std::string& value)
{
    if (paramList_.count(key) == 0) {
        RS_LOGD("RSTimeOutDetector :: SetParam Invaild Key ");
        return;
    }
    int valueInt = atoi(value.c_str());
    if (valueInt <= 0 || valueInt > 1000000) { // 1000000Ms->1000s
        RS_LOGD("RSTimeOutDetector :: SetParam Invaild Value ");
        return;
    }
    timeOutThresholdMs_ = valueInt;
    paramList_[key] = value;
}

void RSTimeOutDetector::SetLoopStartTag()
{
    startTimeStampMs_ = RSEventTimer::GetSysTimeMs();
}

void RSTimeOutDetector::ResSchedDataReport(uint64_t costTimeMs)
{
#ifdef RES_SCHED_ENABLE
    static int32_t lastStatus = 0;
    uint32_t frameRate = FRAME_RATE_NUM;
    uint32_t refreshTimeMs = ceil((float)(1 * 1000) / (float)frameRate); // 1s->1000Ms
    uint32_t thresholdSoonValueMs =
        refreshTimeMs * THRESHOLD_PARA_2 / THRESHOLD_PARA_3; // two-thirds of each refresh time
    std::unordered_map<std::string, std::string> payload;
    payload["uid"] = std::to_string(getuid());
    payload["pid"] = std::to_string(GetRealPid());
    if (costTimeMs <= thresholdSoonValueMs && lastStatus != CLICK_ANIMATION_NORMAL) {
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(RES_TYPE_CLICK_ANIMATION,
            CLICK_ANIMATION_NORMAL, payload);
        lastStatus = CLICK_ANIMATION_NORMAL;
    } else if (costTimeMs > thresholdSoonValueMs && costTimeMs <= refreshTimeMs &&
        lastStatus != CLICK_ANIMATION_SOON) {
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(RES_TYPE_CLICK_ANIMATION,
            CLICK_ANIMATION_SOON, payload);
        lastStatus = CLICK_ANIMATION_SOON;
    } else if (costTimeMs > refreshTimeMs && lastStatus != CLICK_ANIMATION_BOOST) {
        OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(RES_TYPE_CLICK_ANIMATION,
            CLICK_ANIMATION_BOOST, payload);
        lastStatus = CLICK_ANIMATION_BOOST;
    }
#endif
}

void RSTimeOutDetector::SetLoopFinishTag(
    int32_t focusAppPid, int32_t focusAppUid, std::string& focusAppBundleName, std::string& focusAppAbilityName)
{
    uint64_t finishTimeStampMs = RSEventTimer::GetSysTimeMs();
    RS_LOGD("RSTimeOutDetector :: One loop cost Time: %" PRIu64 " ", finishTimeStampMs - startTimeStampMs_);
    if (finishTimeStampMs > startTimeStampMs_) {
        auto durationStampMs = finishTimeStampMs - startTimeStampMs_;
        if (durationStampMs > static_cast<uint64_t>(timeOutThresholdMs_)) {
            focusAppPid_ = focusAppPid;
            focusAppUid_ = focusAppUid;
            focusAppBundleName_ = focusAppBundleName;
            focusAppAbilityName_ = focusAppAbilityName;
            EventReport(durationStampMs);
        }
#ifdef RES_SCHED_ENABLE
        ResSchedDataReport(durationStampMs);
#endif
    }
}

void RSTimeOutDetector::EventReport(uint64_t costTimeMs)
{
    std::string msg = "RS TimeOut: one loop cost " + std::to_string(costTimeMs) + "ms";
    RSSysEventMsg eventMsg = {
        stringId_,
        msg,
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        focusAppPid_,
        focusAppUid_,
        focusAppBundleName_,
        focusAppAbilityName_
    };
    if (eventCallback_ != nullptr) {
        eventCallback_(eventMsg);
    }
}
}
}