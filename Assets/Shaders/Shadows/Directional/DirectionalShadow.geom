#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

layout (std140, binding = 0) uniform LightSpaceMatrices {
    mat4 ViewMatrix;
    mat4 cascadeViewProj[4];
    float cascadeSplits[4];
};

void main() {
    for (int layer = 0; layer < 4; ++layer) {
        gl_Layer = layer;
        for (int i = 0; i < 3; ++i) {
            gl_Position = cascadeViewProj[layer] * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    }
}
