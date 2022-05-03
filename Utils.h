namespace utils {
float linexp(float f, float slo, float shi, float dlo, float dhi) {
    if (f <= slo) {
        return dlo;
    } else if (f >= shi) {
        return dhi;
    }
    return pow(dhi / dlo, (f - slo) / (shi - slo)) * dlo;
}
float linlin(float f, float slo, float shi, float dlo, float dhi) {
    if (f <= slo) {
        return dlo;
    } else if (f >= shi) {
        return dhi;
    }
    return (f - slo) / (shi - slo) * (dhi - dlo) + dlo;
}
float clamp(float f, float slo, float shi) {
    if (f < slo) {
        return slo;
    } else if (f > shi) {
        return shi;
    }
    return f;
}
}
