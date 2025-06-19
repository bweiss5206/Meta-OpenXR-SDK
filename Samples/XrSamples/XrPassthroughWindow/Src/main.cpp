#include <OVR_Math.h>
#include <XrApp.h>
#include <Input/ControllerRenderer.h>
#include "XrPassthroughHelper.h"

class XrPassthroughWindowApp : public OVRFW::XrApp {
public:
    XrPassthroughWindowApp() = default;

    std::vector<const char*> GetExtensions() override {
        std::vector<const char*> exts = XrApp::GetExtensions();
        for (const auto& e : XrPassthroughHelper::RequiredExtensionNames()) {
            exts.push_back(e);
        }
        return exts;
    }

    bool SessionInit() override {
        passthrough_ = std::make_unique<XrPassthroughHelper>(GetInstance());
        passthrough_->SessionInit(GetSession());
        layer_ = passthrough_->CreateProjectedLayer();
        passthrough_->Start();

        leftController_.Init(true);
        rightController_.Init(false);
        return true;
    }

    void SessionEnd() override {
        leftController_.Shutdown();
        rightController_.Shutdown();
        if (passthrough_) {
            passthrough_->DestroyLayer(layer_);
            passthrough_->SessionEnd();
            passthrough_.reset();
        }
    }

    void PreProjectionAddLayer(xrCompositorLayerUnion* layers, int& layerCount) override {
        if (layer_ != XR_NULL_HANDLE) {
            XrCompositionLayerPassthroughFB ptLayer{XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB};
            ptLayer.layerHandle = layer_;
            layers[layerCount++].Passthrough = ptLayer;
        }
    }

    void Update(const OVRFW::ovrApplFrameIn& in) override {
        XrSpace space = GetCurrentSpace();
        XrTime time = ToXrTime(in.PredictedDisplayTime);
        passthrough_->Update(space, time);

        OVR::Posef pose = in.HeadPose;

        if (in.LeftRemoteTracked) {
            leftController_.Update(in.LeftRemotePose);
        }
        if (in.RightRemoteTracked) {
            rightController_.Update(in.RightRemotePose);
        }
    }

    void Render(const OVRFW::ovrApplFrameIn& in, OVRFW::ovrRendererOutput& out) override {
        if (in.LeftRemoteTracked) {
            leftController_.Render(out.Surfaces);
        }
        if (in.RightRemoteTracked) {
            rightController_.Render(out.Surfaces);
        }
    }

private:
    std::unique_ptr<XrPassthroughHelper> passthrough_;
    XrPassthroughLayerFB layer_ = XR_NULL_HANDLE;
    OVRFW::ControllerRenderer leftController_;
    OVRFW::ControllerRenderer rightController_;
};

ENTRY_POINT(XrPassthroughWindowApp)
