#include "Application.h"

#include <iostream>

#include "Engine.h"
#include "Rendering/Shaders/Shader.h"
#include "Rendering/Shaders/ShaderBuilder.h"

int ApplicationMain(Platform::Window* const Window)
{
    Core::Engine::Start(Window);
    Core::ShaderBuilder shaderBuilder;
    Core::Shader shader = shaderBuilder.SetStage("Assets/Shaders/PBR/PBR.vert", Core::ShaderStage::VertexShader)
            .SetStage("Assets/Shaders/PBR/PBR.frag", Core::ShaderStage::FragmentShader)
            .Build();
    while (!Window->ShouldClose())
    {
    }
    Core::Engine::Shutdown();
    return 0;
}
