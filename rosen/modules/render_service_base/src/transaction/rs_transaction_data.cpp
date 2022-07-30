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

#include "transaction/rs_transaction_data.h"

#include "command/rs_command.h"
#include "command/rs_command_factory.h"
#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {

#ifdef ROSEN_OHOS
RSTransactionData* RSTransactionData::Unmarshalling(Parcel& parcel)
{
    auto transactionData = new RSTransactionData();
    if (transactionData->UnmarshallingCommand(parcel)) {
        return transactionData;
    }
    ROSEN_LOGE("RSTransactionData Unmarshalling Failed");
    delete transactionData;
    return nullptr;
}

bool RSTransactionData::Marshalling(Parcel& parcel) const
{
    bool success = true;
    success = success && parcel.WriteInt32(static_cast<int32_t>(payload_.size()));
    for (auto& [nodeId, followType, command] : payload_) {
        success = success && parcel.WriteUint64(nodeId);
        success = success && parcel.WriteUint8(static_cast<uint8_t>(followType));
        success = success && command->Marshalling(parcel);
        if (!success) {
            ROSEN_LOGE("failed RSTransactionData::Marshalling type:%s", command->PrintType().c_str());
            break;
        }
    }
    success = success && parcel.WriteUint64(timestamp_);
    success = success && parcel.WriteInt32(pid_);
    success = success && parcel.WriteUint64(index_);
    success = success && parcel.WriteBool(isUniRender_);
    return success;
}
#endif // ROSEN_OHOS

void RSTransactionData::Process(RSContext& context)
{
    for (auto& [nodeId, followType, command] : payload_) {
        if (command != nullptr) {
            command->Process(context);
        }
    }
}

void RSTransactionData::Clear()
{
    payload_.clear();
    timestamp_ = 0;
}

void RSTransactionData::AddCommand(std::unique_ptr<RSCommand>& command, NodeId nodeId, FollowType followType)
{
    payload_.emplace_back(nodeId, followType, std::move(command));
}

void RSTransactionData::AddCommand(std::unique_ptr<RSCommand>&& command, NodeId nodeId, FollowType followType)
{
    payload_.emplace_back(nodeId, followType, std::move(command));
}

#ifdef ROSEN_OHOS
bool RSTransactionData::UnmarshallingCommand(Parcel& parcel)
{
    Clear();

    int commandSize = 0;
    if (!parcel.ReadInt32(commandSize)) {
        ROSEN_LOGE("RSTransactionData::UnmarshallingCommand cannot read commandSize");
        return false;
    }
    uint8_t followType = 0;
    NodeId nodeId = 0;
    uint16_t commandType = 0;
    uint16_t commandSubType = 0;

    for (int i = 0; i < commandSize; i++) {
        if (!parcel.ReadUint64(nodeId)) {
            ROSEN_LOGE("RSTransactionData::UnmarshallingCommand cannot read nodeId");
            return false;
        }
        if (!parcel.ReadUint8(followType)) {
            ROSEN_LOGE("RSTransactionData::UnmarshallingCommand cannot read followType");
            return false;
        }

        if (!(parcel.ReadUint16(commandType) && parcel.ReadUint16(commandSubType))) {
            return false;
        }
        auto func = RSCommandFactory::Instance().GetUnmarshallingFunc(commandType, commandSubType);
        if (func == nullptr) {
            return false;
        }
        auto command = (*func)(parcel);
        if (command == nullptr) {
            ROSEN_LOGE("failed RSTransactionData::UnmarshallingCommand, type=%d subtype=%d", commandType,
                commandSubType);
            return false;
        }
        payload_.emplace_back(nodeId, static_cast<FollowType>(followType), std::move(command));
    }
    return parcel.ReadUint64(timestamp_) && parcel.ReadInt32(pid_) && parcel.ReadUint64(index_) &&
        parcel.ReadBool(isUniRender_);
}

#endif // ROSEN_OHOS

} // namespace Rosen
} // namespace OHOS
