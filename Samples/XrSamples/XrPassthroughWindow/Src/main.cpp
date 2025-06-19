#include <OVR_Math.h>
#include <XrApp.h>
#include <Input/ControllerRenderer.h>
#include <Render/GeometryRenderer.h>
#include "XrPassthroughHelper.h"
#include <algorithm>


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

        quad_.Init(OVRFW::BuildTesselatedQuadDescriptor(4, 4, false));
        quad_.ChannelControl = {0, 1, 0, 1};
        return true;
    }

    void SessionEnd() override {
        quad_.Shutdown();
        leftController_.Shutdown();
        rightController_.Shutdown();
        if (passthrough_) {
            if (window_ != XR_NULL_HANDLE) {
                passthrough_->DestroyGeometryInstance(window_);
            }
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

        if (window_ == XR_NULL_HANDLE) {
            const std::vector<XrVector3f> verts{
                {-0.5f,-0.5f,0.0f}, {-0.5f,0.5f,0.0f}, {0.5f,0.5f,0.0f}, {0.5f,-0.5f,0.0f}};
            const std::vector<uint32_t> idx{0,1,2,2,3,0};
            window_ = passthrough_->CreateGeometryInstance(layer_, space, verts, idx);
        }

        scale_ += in.RightRemoteJoystick.y * 0.01f;
        scale_ = std::clamp(scale_, 0.2f, 3.0f);
        alpha_ += in.RightRemoteJoystick.x * 0.01f;
        alpha_ = std::clamp(alpha_, 0.0f, 1.0f);

        OVR::Posef pose = in.HeadPose * OVR::Posef(OVR::Quatf(), {0.0f, 0.0f, -1.5f});
        quad_.SetPose(pose);
        quad_.SetScale({scale_, scale_, 1.0f});
        quad_.DiffuseColor.w = alpha_;
        quad_.Update();

        passthrough_->SetGeometryInstanceTransform(
            window_, time, ToXrPosef(pose), {scale_, scale_, 1.0f});

        if (in.LeftRemoteTracked) {
            leftController_.Update(in.LeftRemotePose);
        }
        if (in.RightRemoteTracked) {
            rightController_.Update(in.RightRemotePose);
        }
    }

    void Render(const OVRFW::ovrApplFrameIn& in, OVRFW::ovrRendererOutput& out) override {
        quad_.Render(out.Surfaces);
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
    XrGeometryInstanceFB window_ = XR_NULL_HANDLE;
    float scale_ = 1.0f;
    float alpha_ = 0.8f;
    OVRFW::GeometryRenderer quad_;
    OVRFW::ControllerRenderer leftController_;
    OVRFW::ControllerRenderer rightController_;
};

ENTRY_POINT(XrPassthroughWindowApp)
