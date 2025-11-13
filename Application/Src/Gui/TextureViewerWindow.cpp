#include "TextureViewerWindow.h"

#include <algorithm>
#include <cstdint>
#include <imgui.h>

namespace Application
{
    int32_t TextureViewerWindow::_selectedTextureId = 0;
    int32_t TextureViewerWindow::_imageLayerIndex = 0;

    void TextureViewerWindow::Render(bool& ShowTextureViewer, TextureArraySliceViewer& TextureArraySliceViewer)
    {
        constexpr ImGuiWindowFlags windowFlags =
                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoResize;
        if (ImGui::Begin("Texture Viewer", &ShowTextureViewer, windowFlags))
        {
            constexpr ImVec2 imageSize(512, 512);
            int32_t target = GL_NONE;
            if (!glIsTexture(_selectedTextureId))
            {
                ImGui::InvisibleButton("Texture", imageSize);
            } else
            {
                glGetTextureParameteriv(_selectedTextureId, GL_TEXTURE_TARGET, &target);
                if (target == GL_TEXTURE_2D_ARRAY)
                {
                    int32_t layerCount = 0;
                    glGetTextureLevelParameteriv(_selectedTextureId, 0, GL_TEXTURE_DEPTH, &layerCount);
                    _imageLayerIndex = std::clamp(_imageLayerIndex, 0, layerCount - 1);
                    TextureArraySliceViewer.ShowLayer(_selectedTextureId, _imageLayerIndex, imageSize);
                } else if (target == GL_TEXTURE_2D)
                {
                    ImGui::Image(
                        reinterpret_cast<ImTextureID>(reinterpret_cast<void*>(static_cast<intptr_t>(
                            _selectedTextureId))),
                        imageSize);
                } else
                {
                    ImGui::InvisibleButton("Texture", imageSize);
                }
            }
            ImGui::InputInt("Texture ID", &_selectedTextureId);
            _selectedTextureId = std::max(_selectedTextureId, 0);

            if (target == GL_TEXTURE_2D_ARRAY)
            {
                ImGui::InputInt("Layer", &_imageLayerIndex);
            }
        }
        ImGui::End();
    }
}
