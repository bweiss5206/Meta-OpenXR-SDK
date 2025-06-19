#include <OVR_Math.h>
#include <XrApp.h>
#include <Input/ControllerRenderer.h>
#include <Input/TinyUI.h>
#include <cstring>
#include "XrPassthroughHelper.h"

class XrPassthroughWindowApp : public OVRFW::XrApp {
public:
    XrPassthroughWindowApp() = default;

    bool AppInit(const xrJava* context) override {
        bool ok = OVRFW::XrApp::AppInit(context);
        if (!ok) {
            return false;
        }
        if (ExtensionsArePresent(XrPassthroughHelper::RequiredExtensionNames())) {
            passthrough_ = std::make_unique<XrPassthroughHelper>(GetInstance());
        } else {
            ALOGE("XR_FB_passthrough NOT supported on this runtime");
        }
        return true;
    }

    std::vector<const char*> GetExtensions() override {
        std::vector<const char*> exts = XrApp::GetExtensions();
        for (const auto& e : XrPassthroughHelper::RequiredExtensionNames()) {
            exts.push_back(e);
        }
        return exts;
    }

    bool ExtensionsArePresent(const std::vector<const char*>& list) const {
        const auto props = GetXrExtensionProperties();
        for (const char* ext : list) {
            bool found = false;
            for (const auto& p : props) {
                if (!strcmp(ext, p.extensionName)) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        return true;
    }

    void InitializeUI() {
        if (uiInitialized_) {
            return;
        }
        uiInitialized_ = ui_.Init(GetContext(), GetFileSys());
        if (!uiInitialized_) {
            ALOGE("TinyUI::Init FAILED.");
            return;
        }
        windowObject_ = ui_.AddLabel("", {0.0f, 0.0f, -1.5f}, {400.0f, 300.0f});
        ui_.AddSlider("Size", {-0.4f, -0.6f, -1.5f}, &windowScale_, 1.0f, 0.1f, 0.5f, 2.0f);
        ui_.AddSlider("Alpha", {0.4f, -0.6f, -1.5f}, &windowAlpha_, 1.0f, 0.05f, 0.0f, 1.0f);
    }

    bool SessionInit() override {
        if (passthrough_) {
            passthrough_->SessionInit(GetSession());
            layer_ = passthrough_->CreateReconstructionLayer();
            passthrough_->Start();
        }

        leftController_.Init(true);
        rightController_.Init(false);

        InitializeUI();
        return true;
    }

    void SessionEnd() override {
        leftController_.Shutdown();
        rightController_.Shutdown();
        if (passthrough_) {
            if (layer_ != XR_NULL_HANDLE) {
                passthrough_->DestroyLayer(layer_);
                layer_ = XR_NULL_HANDLE;
            }
            passthrough_->SessionEnd();
            passthrough_.reset();
        }
        if (uiInitialized_) {
            ui_.Shutdown();
            uiInitialized_ = false;
            windowObject_ = nullptr;
        }
    }

    void PreProjectionAddLayer(xrCompositorLayerUnion* layers, int& layerCount) override {
        if (passthrough_ && layer_ != XR_NULL_HANDLE) {
            XrCompositionLayerPassthroughFB ptLayer{XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB};
            ptLayer.layerHandle = layer_;
            layers[layerCount++].Passthrough = ptLayer;
        }
    }

    void Update(const OVRFW::ovrApplFrameIn& in) override {
        XrSpace space = GetCurrentSpace();
        XrTime time = ToXrTime(in.PredictedDisplayTime);
        if (passthrough_) {
            passthrough_->Update(space, time);
        }

        OVR::Posef pose = in.HeadPose;

        if (in.LeftRemoteTracked) {
            leftController_.Update(in.LeftRemotePose);
        }
        if (in.RightRemoteTracked) {
            rightController_.Update(in.RightRemotePose);
        }

        if (windowObject_) {
            windowObject_->SetSurfaceDims(0, {400.0f * windowScale_, 300.0f * windowScale_});
            windowObject_->RegenerateSurfaceGeometry(0, false);
            windowObject_->SetSurfaceColor(0, {0.1f, 0.1f, 0.1f, windowAlpha_});
        }

        ui_.HitTestDevices().clear();
        if (in.LeftRemoteTracked) {
            ui_.AddHitTestRay(in.LeftRemotePointPose, in.LeftRemoteIndexTrigger > 0.25f);
        }
        if (in.RightRemoteTracked) {
            ui_.AddHitTestRay(in.RightRemotePointPose, in.RightRemoteIndexTrigger > 0.25f);
        }
        ui_.Update(in);
    }

    void Render(const OVRFW::ovrApplFrameIn& in, OVRFW::ovrRendererOutput& out) override {
        if (in.LeftRemoteTracked) {
            leftController_.Render(out.Surfaces);
        }
        if (in.RightRemoteTracked) {
            rightController_.Render(out.Surfaces);
        }
        ui_.Render(in, out);
    }

private:
    std::unique_ptr<XrPassthroughHelper> passthrough_;
    XrPassthroughLayerFB layer_ = XR_NULL_HANDLE;
    OVRFW::ControllerRenderer leftController_;
    OVRFW::ControllerRenderer rightController_;

    bool uiInitialized_ = false;
    OVRFW::TinyUI ui_;
    OVRFW::VRMenuObject* windowObject_ = nullptr;
    float windowScale_ = 1.0f;
    float windowAlpha_ = 1.0f;
};

ENTRY_POINT(XrPassthroughWindowApp)
