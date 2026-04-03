#ifndef COMPLEX_INCLUDED
#define COMPLEX_INCLUDED

#include "real.glsl"

#define complex rvec2

complex cmul(complex a, complex b) {
    return complex(
        a.x * b.x - a.y * b.y,
        a.x * b.y + a.y * b.x
    );
}

complex cdiv(complex a, complex b) {
    real denom = dot(b, b); // |b|^2
    return complex(
        dot(a, b) / denom, 
        (a.y * b.x - a.x * b.y) / denom
    );
}

#ifndef USE_DOUBLE // For double, exp, sin, cos do not exist

complex cexp(complex z) {
    return exp(z.x) * complex(cos(z.y), sin(z.y));
}

complex cpow(complex z, real n) {
    real r = length(z);
    real theta = atan(z.y, z.x);
    return pow(r, n) * complex(cos(n * theta), sin(n * theta));
}

#endif

#endif