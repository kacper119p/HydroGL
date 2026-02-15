vec3 ApplyTonemapping(vec3 Color)
{
    const mat3 AcesInputMatrix =
    mat3(vec3(0.59719, 0.35458, 0.04823),
    vec3(0.07600, 0.90834, 0.01566),
    vec3(0.02840, 0.13383, 0.83777));

    const mat3 AcesOutputMatrix =
    mat3(vec3(1.60475, -0.53108, -0.07367),
    vec3(-0.10208, 1.10813, -0.00605),
    vec3(-0.00327, -0.07276, 1.07602));

    Color *= AcesInputMatrix;
    vec3 a = Color * (Color + 0.0245786) - 0.000090537;
    vec3 b = Color * (0.983729 * Color + 0.4329510) + 0.238081;
    Color = a / b;
    return Color * AcesOutputMatrix;
}

float LinearToSrgb(float linearChannel)
{
    return linearChannel <= 0.0031308 ?linearChannel * 12.92 :1.055 * pow(linearChannel, 1.0 / 2.4) - 0.055;
}

vec3 LinearToSrgb(vec3 linearColor)
{
    bvec3 isLower = lessThanEqual(linearColor, vec3(0.0031308));
    vec3 low = linearColor * 12.92;
    vec3 high = 1.055 * pow(linearColor, vec3(1.0 / 2.4)) - 0.055;
    return mix(high, low, isLower);
}

float SrgbToLinear(float SrgbChannel)
{
    return SrgbChannel <= 0.04045 ? SrgbChannel / 12.92 : pow((SrgbChannel + 0.055) / 1.055, 2.4);
}

vec3 SrgbToLinear(vec3 SrgbColor)
{
    bvec3 isLow = lessThanEqual(SrgbColor, vec3(0.04045));
    vec3 low = SrgbColor / 12.92;
    vec3 high = pow((SrgbColor + 0.055) / 1.055, vec3(2.4));

    return mix(high, low, isLow);
}