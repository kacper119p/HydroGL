#version 460 core

layout (quads, equal_spacing, ccw) in;

in vec3 tcPosition[];
out vec2 TexCoord;
out vec3 Position;
out vec3 Normal;
out vec3 Tangent;

uniform vec3 CameraPosition;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform float Time;
uniform float WaveAmplitude = 1.0;
uniform float WaveFrequency = 0.2;
uniform float WaveSpeed = 1.0;

vec3 generateWaves(vec3 position)
{
    float x = position.x;
    float z = position.z;

    float y = 0.0;

    y += sin(x * WaveFrequency + Time * WaveSpeed) *
    sin(z * WaveFrequency * 0.8 + Time * WaveSpeed * 0.7) *
    WaveAmplitude;

    y += sin(x * WaveFrequency * 0.4 + Time * WaveSpeed * 0.8) *
    sin(z * WaveFrequency * 0.6 + Time * WaveSpeed * 0.9) *
    WaveAmplitude * 0.7;

    return vec3(position.x, y, position.z);
}

void computeDerivatives(vec3 position, out float dydx, out float dydz)
{
    float x = position.x;
    float z = position.z;

    float A1 = WaveAmplitude;
    float w1x = WaveFrequency;
    float w1z = WaveFrequency * 0.8;
    float phi1x = Time * WaveSpeed;
    float phi1z = Time * WaveSpeed * 0.7;

    float A2 = WaveAmplitude * 0.7;
    float w2x = WaveFrequency * 0.4;
    float w2z = WaveFrequency * 0.6;
    float phi2x = Time * WaveSpeed * 0.8;
    float phi2z = Time * WaveSpeed * 0.9;

    dydx = w1x * A1 * cos(x * w1x + phi1x) * sin(z * w1z + phi1z) +
    w2x * A2 * cos(x * w2x + phi2x) * sin(z * w2z + phi2z);

    dydz = w1z * A1 * sin(x * w1x + phi1x) * cos(z * w1z + phi1z) +
    w2z * A2 * sin(x * w2x + phi2x) * cos(z * w2z + phi2z);
}

void main()
{
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec3 p0 = mix(tcPosition[0], tcPosition[1], u);
    vec3 p1 = mix(tcPosition[3], tcPosition[2], u);
    vec3 position = mix(p0, p1, v);

    vec3 finalPosition = generateWaves(position);

    float delta = 0.01;
    vec3 tangentPoint = generateWaves(position + vec3(delta, 0.0, 0.0));
    vec3 bitangentPoint = generateWaves(position + vec3(0.0, 0.0, delta));

    float dydx, dydz;
    computeDerivatives(position, dydx, dydz);
    vec3 tangent = normalize(vec3(1.0, dydx, 0.0));
    vec3 bitangent = normalize(vec3(0.0, dydz, 1.0));
    Normal = normalize(cross(bitangent, tangent));
    Tangent = tangent;

    Position = finalPosition;
    TexCoord = vec2(u, v);

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(finalPosition, 1.0);
}