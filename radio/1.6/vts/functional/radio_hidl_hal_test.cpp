/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <radio_hidl_hal_utils_v1_6.h>

void RadioHidlTest_v1_6::SetUp() {
    radio_v1_6 = android::hardware::radio::V1_6::IRadio::getService(GetParam());
    ASSERT_NE(nullptr, radio_v1_6.get());

    radioRsp_v1_6 = new (std::nothrow) RadioResponse_v1_6(*this);
    ASSERT_NE(nullptr, radioRsp_v1_6.get());

    count_ = 0;

    radioInd_v1_6 = new (std::nothrow) RadioIndication_v1_6(*this);
    ASSERT_NE(nullptr, radioInd_v1_6.get());

    radio_v1_6->setResponseFunctions(radioRsp_v1_6, radioInd_v1_6);

    getDataCallList();
    EXPECT_EQ(RadioResponseType::SOLICITED, radioRsp_v1_6->rspInfo.type);
    EXPECT_EQ(serial, radioRsp_v1_6->rspInfo.serial);
    EXPECT_EQ(::android::hardware::radio::V1_6::RadioError::NONE, radioRsp_v1_6->rspInfo.error);

    sp<::android::hardware::radio::config::V1_1::IRadioConfig> radioConfig =
            ::android::hardware::radio::config::V1_1::IRadioConfig::getService();
    /* Enforce Vts testing with RadioConfig is existed. */
    ASSERT_NE(nullptr, radioConfig.get());

    /* Enforce Vts Testing with Sim Status Present only. */
    EXPECT_EQ(CardState::PRESENT, cardStatus.base.base.base.cardState);
}

/*
 * Notify that the response message is received.
 */
void RadioHidlTest_v1_6::notify(int receivedSerial) {
    std::unique_lock<std::mutex> lock(mtx_);
    if (serial == receivedSerial) {
        count_++;
        cv_.notify_one();
    }
}

/*
 * Wait till the response message is notified or till TIMEOUT_PERIOD.
 */
std::cv_status RadioHidlTest_v1_6::wait() {
    std::unique_lock<std::mutex> lock(mtx_);

    std::cv_status status = std::cv_status::no_timeout;
    auto now = std::chrono::system_clock::now();
    while (count_ == 0) {
        status = cv_.wait_until(lock, now + std::chrono::seconds(TIMEOUT_PERIOD));
        if (status == std::cv_status::timeout) {
            return status;
        }
    }
    count_--;
    return status;
}

void RadioHidlTest_v1_6::getDataCallList() {
    serial = GetRandomSerialNumber();
    radio_v1_6->getDataCallList_1_6(serial);
    EXPECT_EQ(std::cv_status::no_timeout, wait());
}
