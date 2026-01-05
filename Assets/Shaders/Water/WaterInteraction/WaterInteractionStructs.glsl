struct triangleProperties
{
    vec3 Drag;
    float SubmergedArea;
    vec3 CenterOfImmersion;
    float SubmergedVolume;
    vec3 CenterOfAir;
    float AirVolume;
    vec3 AirDrag;
};

struct intersectionSegment
{
    vec2 P0;
    vec2 P1;
};
