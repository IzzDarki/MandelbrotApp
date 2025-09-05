#ifndef RK45_INCLUDED
#define RK45_INCLUDED

#include "real.glsl"

// Problem definition must contain this
// const real t0 = 0.0;
// const real t_end = 10.0;
// rrvec4 rhs(rrvec4 y0) {
//     return ...;
// }

// I trust the glsl compilation to optimize const float operations away
// https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta%E2%80%93Fehlberg_method, Formula 1
const real a21 = 2.0/9.0;
const real a31 = 1.0/12.0;       const real a32 = 1.0/4.0;
const real a41 = 69.0/128.0;     const real a42 = -243.0/128.0;   const real a43 = 135.0/64.0;
const real a51 = -17.0/12.0;     const real a52 = 27.0/4.0;       const real a53 = -27.0/5.0;    const real a54 = 16.0/15.0;
const real a61 = 65.0/432.0;     const real a62 = -5.0/16.0;      const real a63 = 13.0/16.0;    const real a64 = 4.0/27.0;     const real a65 = 5.0/144.0;

const real b1 = 47.0/450.0; const real b2 = 0.0;  const real b3 = 12.0/25.0; const real b4 = 32.0/225.0; const real b5 = 1.0/30.0; const real b6 = 6.0/25.0;
const real b1_ = 1.0/9.0;   const real b2_ = 0.0; const real b3_ = 9.0/20.0; const real b4_ = 16.0/45.0; const real b5_ = 1.0/12.0; const real b6_ = 0.0;

const real tau_fac_min = 1.0/3.0; const real tau_fac_max = 2.0; // tau_fac_max between 1.5 and 5.0 according to Numerik-Skript
const real tau_fac = 0.9; // tau_fac = 0.8 or 0.9 or pow(0.25, 1.0/p), where p = 5.0 in our case according to Numerik-Skript

//const real rtol = 1e-8;
//const real atol = 1e-8; // must be non-zero
uniform float rtol;
uniform float atol; // must be non-zero

// const uint MAX_STEPS = 10000u;
uniform uint MAX_STEPS;
const uint MAX_SAME_STEPS = 22u;
const real MIN_TAU = 1e-5;

const uint SUCCESS = 0u;
const uint ERR_TOO_MANY_STEPS = 1u;
const uint ERR_TOO_MANY_SAME_STEPS = 2u;
const uint ERR_TAU_TOO_SMALL = 3u;

real scaled_norm(rvec4 vector, rvec4 scale_vec) {
    rvec4 temp = vector / scale_vec;
    return 0.5 * length(temp); // length is 2-norm // 0.5 = sqrt(1.0/dimensionality of ode)
}

rvec4 rk45_step(rvec4 y0, inout real tau, out bool isAccepted) {
    rvec4 Y1 = y0;
    rvec4 Z1 = rhs(Y1);

    rvec4 Y2 = y0 + tau*(a21*Z1);
    rvec4 Z2 = rhs(Y2);

    rvec4 Y3 = y0 + tau*(a31*Z1 + a32*Z2);
    rvec4 Z3 = rhs(Y3);

    rvec4 Y4 = y0 + tau*(a41*Z1 + a42*Z2 + a43*Z3);
    rvec4 Z4 = rhs(Y4);

    rvec4 Y5 = y0 + tau*(a51*Z1 + a52*Z2 + a53*Z3 + a54*Z4);
    rvec4 Z5 = rhs(Y5);

    rvec4 Y6 = y0 + tau*(a61*Z1 + a62*Z2 + a63*Z3 + a64*Z4 + a65*Z5);
    rvec4 Z6 = rhs(Y6);

    rvec4 y1 = y0 + tau*(b1*Z1 + b2*Z2 + b3*Z3 + b4*Z4 + b5*Z5 + b6*Z6);
    rvec4 err_vec = tau*((b1 - b1_)*Z1 + (b2 - b2_)*Z2 + (b3 - b3_)*Z3 + (b4 - b4_)*Z4 + (b5 - b5_)*Z5 + (b6 - b6_)*Z6);


    // Error estimation and Calculation of optimal tau
    rvec4 scale_vec = rvec4(atol) + max(abs(y0), abs(y1))*rvec4(rtol);
    real err = scaled_norm(err_vec, scale_vec);
    tau = tau * min(tau_fac_max, max(tau_fac_min, tau_fac*pow(1.0 / max(err, 1e-10), 1.0/5.0))); // max(err, 1e-10) to prevent division by zero // 5.0 = order of rk5

    if (err >= 1.0) {
        isAccepted = false;
        return y0;
    }

    isAccepted = true;
    return y1;
}

rvec4 rk45(rvec4 y0, out uint error_code, out uint step_counter, out uint same_step_counter) {
    // Guess initial step size (according to ChatGPT)
    rvec4 z0 = rhs(y0);
    real y0_norm = scaled_norm(y0, rvec4(atol) + abs(y0)*rvec4(rtol));
    real z0_norm = scaled_norm(z0, rvec4(atol) + abs(z0)*rvec4(rtol));
    real tau = 0.01 * y0_norm / max(z0_norm, 1e-10); // prevent divide by 0.0
    tau = min(1.0, max(1e-8, tau)); // clamp to range [1e-8, 1.0]

    step_counter = 0;
    same_step_counter = 0;
    real t = t0;
    rvec4 y = y0;
    while(t < t_end - 1e-9) { // small tolerance to rounding errors
        if (step_counter >= MAX_STEPS) {
            error_code = ERR_TOO_MANY_STEPS;
            return rvec4(0.0);
        }
        if (same_step_counter >= MAX_SAME_STEPS) {
            error_code = ERR_TOO_MANY_SAME_STEPS;
            return rvec4(0.0);
        }
        if (tau < MIN_TAU && t + tau < t_end - 1e-9) { // tau too small and not close to end
            error_code = ERR_TAU_TOO_SMALL;
            return rvec4(0.0);
        }

        bool isAccepted;
        tau = min(tau, t_end - t); // make sure not to overshoot t_end
        real used_tau = tau;
        y = rk45_step(y, tau, isAccepted); // tau is always updated to new step size. On success next y is returned, otherwise the initial y

        if (isAccepted) {
            same_step_counter = 0;
            t += used_tau;
        } else {
            same_step_counter += 1;
        }

        step_counter += 1;
    }

    error_code = SUCCESS;
    return y;
}

#endif
