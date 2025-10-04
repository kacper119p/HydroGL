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
