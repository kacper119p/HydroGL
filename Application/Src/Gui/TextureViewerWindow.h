#pragma once
#include <cstdint>

#include "Gui/TextureArraySliceViewer.h"

namespace Application
{
    class TextureViewerWindow
    {
    private:
        static int32_t _selectedTextureId;
        static int32_t _imageLayerIndex;

    public:
        static void Render(bool& ShowTextureViewer, TextureArraySliceViewer& TextureArraySliceViewer);
    };
}
