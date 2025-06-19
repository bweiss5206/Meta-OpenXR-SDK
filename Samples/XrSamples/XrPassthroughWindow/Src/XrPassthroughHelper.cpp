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
Filename    :   XrPassthroughHelper.cpp
Content     :   Helper inteface for openxr XR_FB_spatial_entity and related extensions
Created     :   September 2023
Authors     :   Adam Bengis, Peter Chan
Language    :   C++
Copyright   :   Copyright (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
************************************************************************************************/

#include "XrPassthroughHelper.h"

#include <Misc/Log.h>

std::vector<const char*> XrPassthroughHelper::RequiredExtensionNames() {
    return {XR_FB_PASSTHROUGH_EXTENSION_NAME};
}

XrPassthroughHelper::XrPassthroughHelper(XrInstance instance) : XrHelper(instance) {
    // XR_FB_passthrough
    OXR(xrGetInstanceProcAddr(
        instance,
        "xrCreatePassthroughFB",
        reinterpret_cast<PFN_xrVoidFunction*>(&XrCreatePassthroughFB)));
    OXR(xrGetInstanceProcAddr(
        instance,
        "xrDestroyPassthroughFB",
        reinterpret_cast<PFN_xrVoidFunction*>(&XrDestroyPassthroughFB)));
    OXR(xrGetInstanceProcAddr(
        instance,
        "xrPassthroughStartFB",
        reinterpret_cast<PFN_xrVoidFunction*>(&XrPassthroughStartFB)));
    OXR(xrGetInstanceProcAddr(
        instance,
        "xrPassthroughPauseFB",
        reinterpret_cast<PFN_xrVoidFunction*>(&XrPassthroughPauseFB)));
    OXR(xrGetInstanceProcAddr(
        instance,
        "xrCreatePassthroughLayerFB",
        reinterpret_cast<PFN_xrVoidFunction*>(&XrCreatePassthroughLayerFB)));
    OXR(xrGetInstanceProcAddr(
        instance,
        "xrDestroyPassthroughLayerFB",
        reinterpret_cast<PFN_xrVoidFunction*>(&XrDestroyPassthroughLayerFB)));
    OXR(xrGetInstanceProcAddr(
        instance,
        "xrPassthroughLayerSetStyleFB",
        reinterpret_cast<PFN_xrVoidFunction*>(&XrPassthroughLayerSetStyleFB)));
}

void XrPassthroughHelper::SessionInit(XrSession session) {
    Session = session;

    XrPassthroughCreateInfoFB passthroughInfo{XR_TYPE_PASSTHROUGH_CREATE_INFO_FB};
    OXR(XrCreatePassthroughFB(Session, &passthroughInfo, &Passthrough));
}

void XrPassthroughHelper::SessionEnd() {
    OXR(XrDestroyPassthroughFB(Passthrough));

    Session = XR_NULL_HANDLE;
}

void XrPassthroughHelper::Update(XrSpace currentSpace, XrTime predictedDisplayTime) {}

XrPassthroughLayerFB XrPassthroughHelper::CreateReconstructionLayer() const {
    XrPassthroughLayerFB layer = XR_NULL_HANDLE;

    XrPassthroughLayerCreateInfoFB layerInfo{XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB};
    layerInfo.passthrough = Passthrough;
    layerInfo.purpose = XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;
    layerInfo.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;
    XrResult result = XrCreatePassthroughLayerFB(Session, &layerInfo, &layer);
    if (result != XR_SUCCESS) {
        ALOGE("xrCreatePassthroughLayerFB failed, error %d", result);
        return XR_NULL_HANDLE;
    }

    XrPassthroughStyleFB style{XR_TYPE_PASSTHROUGH_STYLE_FB};
    style.textureOpacityFactor = 1.0f;
    style.edgeColor = {0.0f, 0.0f, 0.0f, 0.0f};
    result = XrPassthroughLayerSetStyleFB(layer, &style);
    if (result != XR_SUCCESS) {
        ALOGE("xrPassthroughLayerSetStyleFBfailed, error %d", result);
        return XR_NULL_HANDLE;
    }

    return layer;
}

void XrPassthroughHelper::DestroyLayer(XrPassthroughLayerFB layer) const {
    OXR(XrDestroyPassthroughLayerFB(layer));
}


void XrPassthroughHelper::Start() const {
    OXR(XrPassthroughStartFB(Passthrough));
}

void XrPassthroughHelper::Pause() const {
    OXR(XrPassthroughPauseFB(Passthrough));
}
