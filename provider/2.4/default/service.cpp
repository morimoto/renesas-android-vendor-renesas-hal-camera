/*
 * Copyright 2017 The Android Open Source Project
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

#define LOG_TAG "android.hardware.camera.provider@2.4-service.kingfisher"

#include <android-base/logging.h>
#include <android/hardware/camera/provider/2.4/ICameraProvider.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>

#include "CameraProvider.h"

using android::hardware::camera::provider::V2_4::ICameraProvider;
using android::hardware::camera::provider::V2_4::implementation::CameraProvider;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

int main()
{
    ALOGI("Camera provider Service is starting.");
    // The camera HAL may communicate to other vendor components via
    // /dev/vndbinder
    android::ProcessState::initWithDriver("/dev/vndbinder");
    android::sp<ICameraProvider> camera_provider_hal = new CameraProvider();

    configureRpcThreadpool(6, true);

    const auto status = camera_provider_hal->registerAsService("legacy/0");
    CHECK_EQ(status, android::OK) << "Failed to register Camera Provider HAL.";

    joinRpcThreadpool();

    return EXIT_FAILURE;
}
