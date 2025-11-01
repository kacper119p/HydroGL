vec2 ComplexProduct(const vec2 a, const vec2 b)
{
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

vec2 MultiplyByI(const vec2 x){
    return vec2(-x.y, x.x);
}

vec2 MultiplyByMinusI(const vec2 x){
    return vec2(x.y, -x.x);
}

vec2 Euler(const float x){
    return vec2(cos(x), sin(x));
}

vec2 Conjugate(const vec2 x)
{
    return vec2(x.x, -x.y);
}
