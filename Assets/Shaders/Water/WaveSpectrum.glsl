uniform vec2 WindVelocity = vec2(3.0, 4.0);
uniform float Fetch = 100.0;
uniform float Delta = 0.5;

const float GAMMA = 3.3;
const float G = 9.80665;
const float G2 = G * G;
const float PI = 3.141592653589793;

float CalculateSigma(float Omega, float OmegaP)
{
    if (Omega <= OmegaP)
    {
        return 0.07;
    }
    return 0.09;
}

float CalculateOmegaP(float WindSpeed, float Fetch)
{
    return 22.0 * G2 / WindSpeed / Fetch;
}

float calulateR(float Omega)
{
    float OmegaP = CalculateOmegaP(length(WindVelocity), Fetch);
    float sigma = CalculateSigma(Omega, OmegaP);

    float numerator = Omega - OmegaP;
    float denominator = 2.0 * sigma * sigma * OmegaP * OmegaP;

    return exp(-1.0 * (numerator * numerator) / denominator);
}

float CalculateTheta(vec2 k)
{
    float theta0 = atan(WindVelocity.y, WindVelocity.x);
    return atan(k.y, k.x) - theta0;
}

float CalculateBs(float R)
{
    if (R < 0.95)
    {
        return 2.61 * pow(R, 1.3);
    }
    if (R < 1.6)
    {
        return 2.28 * pow(R, -1.3);
    }
    float epsilon = 0.8393 * exp (-0.567 * log(R * R)) - 0.4;
    return pow(10.0, epsilon);

}

float CalculateQdbe(float R)
{
    if (R < 0.94)
    {
        return (7.1467551 * R - 13.4662001) * R + 7.75651088;
    }
    if (R < 5)
    {
        return (-0.69906109 * R + 0.77975933) * R + 0.10169164;
    }
    if (R < 100)
    {
        return (-2.1860997 * R + 0.0269209) * R + 0.00016283;
    }
    return 1.2038847 * R + 0.0008147;
}

float CalculateQdb(float Bs)
{
    return 1.0 / tanh(Bs * PI);
}

float CalculateDdb(float Qdb, float Bs, float Theta)
{
    float sech = 1.0 / cosh(Bs * Theta);
    return 0.5 * Qdb * Bs * sech * sech;
}

float CalculateDdbe(float Qdbe, float Ddb, float De)
{
    return Qdbe * Ddb * De;
}

float CalculateDe(float Theta, float E, float R)
{
    return pow(abs(cos(Theta / 2.0)), 32.0 * tanh(1.0 / R)) * E * E;
}

float CalculateSpectrum(vec2 k, float E){
    float k_length = length(k);
    if (k_length < 0.00005) return 0.0;

    float theta = CalculateTheta(k);
    float r = calulateR(k_length);
    float bs = CalculateBs(r);
    float qdb = CalculateQdb(bs);

    float de = CalculateDe(theta, E, r);
    float ddb = CalculateDdb(qdb, bs, theta);
    float Qdbe = CalculateQdbe(r);

    float ddbe = CalculateDdbe(Qdbe, ddb, de);
    return (1 - Delta) / 2.0 / PI + Delta * ddbe;
}

