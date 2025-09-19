#ifndef N_BODY_PROBLEM_INCLUDED
#define N_BODY_PROBLEM_INCLUDED

#define N 3 // Number of bodies
#define D 3 // Dimension of space
#define M 2*N // M is number of array elements should rk45_arr works with
#include "real.glsl"

// Unit scaling
const real time_scale = 60.0*60.0*24.0; // We use time_scale * s as time unit (in this case this equals days)
const real mass_scale = 1e22;           // We use mass_scale * kg as mass unit
const real length_scale = 1e7;          // We use length_scale * m as length unit

// N-Body Problem Parameters
// uniform real g; // Gravitational constant
const real g = mass_scale / (pow(length_scale, 3.0)) * pow(time_scale, 2.0) * 6.67384e-11; // unit [(length_scale*m)^3 / (mass_scale*kg * (time_scale*s)^2)]
uniform real m[N]; // Start masses of all N bodies
// uniform rvecd q0[N]; // Start positions of all N bodies
// uniform rvecd p0[N]; // Start momenta of all N bodies
const real t0 = 0.0;
uniform float t_end;



real H(rvecd q[N], rvecd p[N]) {
    real h = 0.0;
    for (int i = 0; i < N; ++i) {
        h += 0.5 * dot(p[i], p[i]) / m[i];
        for (int j = 0; j < i; ++j) {
            h -= g * m[i] * m[j] / length(q[i] - q[j]);
        }
    }
    return h;
}

real T(rvecd p[N]) {
    real t = 0.0;
    for (int i = 0; i < N; ++i) {
        t += 0.5 * dot(p[i], p[i]) / m[i];
    }
    return t;
}

real U(rvecd q[N]) {
    real u = 0.0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < i; ++j) {
            u -= g * m[i] * m[j] / length(q[i] - q[j]);
        }
    }
    return u;
}

// Derivative of T
void dT(rvecd p[N], out rvecd dT_out[N]) {
    for (int i = 0; i < N; ++i) {
        dT_out[i] = p[i] / m[i];
    }
}

// Negative derivative of U
void dnU(rvecd q[N], out rvecd dnU_out[N]) {
    // Set dnU_out to zero
    for (int i = 0; i < N; ++i) {
        dnU_out[i] = rvecd(0.0);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < i; ++j) {
            // Calculate force between m[i] and m[j] (pointing to m[j])
            rvecd diff = q[j] - q[i];
            rvecd force = diff * g * m[i] * m[j] / pow(length(diff), 3.0);

            // Add forces
            dnU_out[i] += force;
            dnU_out[j] -= force; // in opposite direction
        }
    }
}

void rhs_arr(rvecd y[2*N], out rvecd rhs_out[2*N]) {
    // dT in first N components
    for (int i = 0; i < N; ++i) {
        rhs_out[i] = y[N+i] / m[i]; // p[i] = y[N+i]
    }

    // dnU in last N components
    for (int i = 0; i < N; ++i) {
        rhs_out[N+i] = rvecd(0.0);
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < i; ++j) {
            // Calculate force between m[i] and m[j] (pointing to m[j])
            rvecd diff = y[j] - y[i];
            rvecd force = diff * g * m[i] * m[j] / pow(length(diff), 3);

            // Add forces
            rhs_out[N+i] += force;
            rhs_out[N+j] -= force; // in opposite direction
        }
    }
}

#endif