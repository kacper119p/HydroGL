#pragma once
#include "ImGuiContext.h"
#include "Input.h"
#include "Gui/TextureArraySliceViewer.h"
#include "Ocean/WaterSystem.h"

namespace Application
{
    class Gui
    {
        Core::WaterSystem* _waterSystem = nullptr;
        std::unique_ptr<TextureArraySliceViewer> _textureArraySliceViewer;
        uint32_t _windowWidth;
        uint32_t _windowHeight;
        Platform::Input::CallbackId _showGuiCallbackId = 0;
        int32_t _textureSliceViewerSelectedTexture = 0;
        int32_t _textureSliceViewerLayerIndex = 0;
        bool _showGui = true;
        bool _showWaterSettings = false;
        bool _showSystemInfo = false;
        bool _showTextureViewer = false;

    public:
        explicit Gui(const Platform::Window& Window, Core::WaterSystem& WaterSystem);

    public:
        ~Gui();

        void Render();

    private:
        void RenderTopBar();

        void RenderWaterSettings();

        void RenderSystemInfoWindow();

        void RenderTextureViewer();
    };
}
