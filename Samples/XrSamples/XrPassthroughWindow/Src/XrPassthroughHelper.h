/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * Licensed under the Oculus SDK License Agreement (the "License");
 * you may not use the Oculus SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.
 *
 * You may obtain a copy of the License at
 * https://developer.oculus.com/licenses/oculussdk/
 *
 * Unless required by applicable law or agreed to in writing, the Oculus SDK
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/************************************************************************************************
Filename    :   XrPassthroughHelper.h
Content     :   Helper inteface for openxr XR_FB_passthrough and related extensions
Created     :   September 2023
Authors     :   Adam Bengis, Peter Chan
Language    :   C++
Copyright   :   Copyright (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
************************************************************************************************/

#pragma once

#include <openxr/openxr.h>

#include "XrHelper.h"

class XrPassthroughHelper : public XrHelper {
   public:
    static std::vector<const char*> RequiredExtensionNames();

   public:
    explicit XrPassthroughHelper(XrInstance instance);

    void SessionInit(XrSession session) override;
    void SessionEnd() override;
    void Update(XrSpace currentSpace, XrTime predictedDisplayTime) override;

    XrPassthroughLayerFB CreateReconstructionLayer() const;
    void DestroyLayer(XrPassthroughLayerFB layer) const;

    void Start() const;
    void Pause() const;

   private:
    XrSession Session = XR_NULL_HANDLE;

    // XR_FB_passthrough
    PFN_xrCreatePassthroughFB XrCreatePassthroughFB = nullptr;
    PFN_xrDestroyPassthroughFB XrDestroyPassthroughFB = nullptr;
    PFN_xrPassthroughStartFB XrPassthroughStartFB = nullptr;
    PFN_xrPassthroughPauseFB XrPassthroughPauseFB = nullptr;
    PFN_xrCreatePassthroughLayerFB XrCreatePassthroughLayerFB = nullptr;
    PFN_xrDestroyPassthroughLayerFB XrDestroyPassthroughLayerFB = nullptr;
    PFN_xrPassthroughLayerSetStyleFB XrPassthroughLayerSetStyleFB = nullptr;
    XrPassthroughFB Passthrough = XR_NULL_HANDLE;
};
