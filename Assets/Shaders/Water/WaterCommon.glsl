#include "Constants.glsl"

const float GAMMA = 3.3;
const float G = 9.80665;
const float G2 = G * G;

struct cascadeSettings
{
    float WaveLengthLowerBound;
    float WaveLengthUpperBound;
    int Length;
    float Padding;
};

float CalculateDispersion(const float k)
{
    return sqrt(G * k);
}
