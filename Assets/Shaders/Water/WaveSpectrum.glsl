uniform vec2 WindVelocity = vec2(1.0, 1.0);
uniform float Fetch = 1000.0;
uniform float Delta = 0.5;
uniform float Swell = 0.5;

const float GAMMA = 3.3;
const float G = 9.80665;
const float G2 = G * G;
const float PI = 3.141592653589793;

float CalculateOmega(float k)
{
    return sqrt(G * k);
}

float CalculateSigma(float Omega, float OmegaP)
{
    return (Omega <= OmegaP) ? 0.07 : 0.09;
}

float CalculateAlpha()
{
    const float windSpeed2 = dot(WindVelocity, WindVelocity);
    return 0.076 * pow(windSpeed2/ (Fetch * G), 0.22);
}

float CalculateOmegaP(float WindSpeed, float Fetch)
{
    return 22.0 * G2 / (WindSpeed * Fetch);
}

float calulateR(float Omega, float OmegaP)
{
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
        return max(2.61 * pow(R, 1.3), 0.00005);
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
    if (R < 5.0)
    {
        return (-0.69906109 * R + 0.77975933) * R + 0.10169164;
    }
    if (R < 100.0)
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

float CalculateDe(float Theta, float R)
{
    return pow(abs(cos(Theta / 2.0)), 32.0 * tanh(1.0 / R) * Swell * Swell);
}

float Jonswap(float alpha, float omega, float wp, float r)
{
    float omega2 = omega * omega;
    float omega5 = omega2 * omega2 * omega;

    float omegaPByOmega = wp / omega;
    float omegaPByOmega2 = omegaPByOmega * omegaPByOmega;
    float omegaPByOmega4 = omegaPByOmega2 * omegaPByOmega2;

    return alpha * G2 / omega5 * exp(-5.0 / 4.0 * omegaPByOmega4) * pow(GAMMA, r);
}

float CalculateSpectrum(vec2 k, float CascadeSize)
{
    float k_length = length(k);
    k_length = max(k_length, 0.00005);

    float omega = CalculateOmega(k_length);
    float omegaP = CalculateOmegaP(length(WindVelocity), Fetch);
    float alpha = CalculateAlpha();
    float theta = CalculateTheta(k);
    float r = calulateR(omega, omegaP);
    float bs = CalculateBs(r);
    float qdb = CalculateQdb(bs);

    float de = CalculateDe(theta, r);
    float ddb = CalculateDdb(qdb, bs, theta);
    float Qdbe = CalculateQdbe(r);

    float ddbe = CalculateDdbe(Qdbe, ddb, de);
    float frequencySpectrum = Jonswap(alpha, omega, omegaP, r);
    float directionalSpectrum = (1 - Delta) / 2.0 / PI + Delta * ddbe;
    //float directionalSpectrum = 1.0 / (2.0 * PI);
    return max(sqrt(4 * PI / CascadeSize * frequencySpectrum * directionalSpectrum * abs(G / (2 * omega))), 0.0);
}

