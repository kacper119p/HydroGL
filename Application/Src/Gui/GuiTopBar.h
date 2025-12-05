#pragma once

namespace Application
{
    class GuiTopBar
    {
    public:
        static void Render(bool& ShowWaterSettings, bool& ShowSystemInfo,
                           bool& ShowTextureViewer);
    };
}
