#include "Constants.glsl"

const float GAMMA = 3.3;
const float G = 9.80665;
const float G2 = G * G;

#define DEFINE_CASCADE_SETTINGS_BUFFER(binding_point) \
    layout(std430, binding = binding_point) readonly restrict buffer cascadeSettingsBuffer \
    { \
        int TextureResolution; \
        int CascadeCount; \
        int VelocityLayers; \
        float VelocityMinHeight; \
        float VelocityMaxHeight; \
        float Padding[3]; \
        cascadeSettings CascadeSettings[]; \
 } CascadeSettingsBuffer;\

#define DEFINE_FOAM_PARAMETERS_BUFFER(binding_point) \
    layout(std140, binding = binding_point) uniform foamParameters \
    { \
        float FoamThreshold; \
        float FoamExponent; \
        float FoamIntensity; \
 } FoamParameters;

struct cascadeSettings
{
    float WaveLengthLowerBound;
    float WaveLengthUpperBound;
    float Length;
    float Padding;
};

float CalculateDispersion(const float k)
{
    return sqrt(G * k);
}
