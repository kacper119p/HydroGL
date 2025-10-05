vec3 BlendNormalsWhiteout(vec3 n1, vec3 n2)
{
    return normalize(vec3(n1.xy + n2.xy, n1.z*n2.z));
}

vec3 BlendNormalsLinear(vec3 n1, vec3 n2)
{
    return normalize(vec3(n1.xy + n2.xy, n1.z*n2.z));
}

vec3 BlendNormals(vec3 n1, vec3 n2)
{
    return BlendNormalsWhiteout(n1, n2);
}

vec3 NormalFromSlope(vec2 slope)
{
    return normalize(vec3(-slope.x, 1.0, -slope.y));
}

vec2 SlopeFromNormal(vec3 normal)
{
    return vec2(-normal.x / normal.y, -normal.z / normal.y);
}
