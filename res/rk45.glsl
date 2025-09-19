#ifndef RK45_INCLUDED
#define RK45_INCLUDED

#include "real.glsl"

// A Problem definition must be included before this and must contain the following
//  #define D ... (defining dimension of vectors rvecd)
//  #define M ... (defining length of arrays used)      (only needed when using rk45_arr, otherwise use #define RK45_DISABLE_ARR_METHODS)
// - rvecd rhs(rvecd y0) { return ... }                 (only when using rk45, otherwise use #define RK45_DISABLE_VEC_METHODS)
// - void rhs_arr(rvecd y0[M], out rvecd y1[M]) { ... } (only when using rk45_arr, otherwise use #define RK45_DISABLE_ARR_METHODS)


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
const real MIN_TAU = 1e-12;

const uint SUCCESS = 0u;
const uint ERR_TOO_MANY_STEPS = 1u;
const uint ERR_TOO_MANY_SAME_STEPS = 2u;
const uint ERR_TAU_TOO_SMALL = 3u;

// * Vector methods (taking a single D-dimensional vector)
#ifndef RK45_DISABLE_VEC_METHODS

real scaled_norm(rvecd vector, rvecd scale_vec) {
    rvecd temp = vector / scale_vec;
    return sqrt(1.0 / D) * length(temp); // length is 2-norm 
}

rvecd rk45_step(rvecd y0, inout real tau, out bool isAccepted) {
    rvecd Y1 = y0;
    rvecd Z1 = rhs(Y1);

    rvecd Y2 = y0 + tau*(a21*Z1);
    rvecd Z2 = rhs(Y2);

    rvecd Y3 = y0 + tau*(a31*Z1 + a32*Z2);
    rvecd Z3 = rhs(Y3);

    rvecd Y4 = y0 + tau*(a41*Z1 + a42*Z2 + a43*Z3);
    rvecd Z4 = rhs(Y4);

    rvecd Y5 = y0 + tau*(a51*Z1 + a52*Z2 + a53*Z3 + a54*Z4);
    rvecd Z5 = rhs(Y5);

    rvecd Y6 = y0 + tau*(a61*Z1 + a62*Z2 + a63*Z3 + a64*Z4 + a65*Z5);
    rvecd Z6 = rhs(Y6);

    rvecd y1 = y0 + tau*(b1*Z1 + b2*Z2 + b3*Z3 + b4*Z4 + b5*Z5 + b6*Z6);
    rvecd err_vec = tau*((b1 - b1_)*Z1 + (b2 - b2_)*Z2 + (b3 - b3_)*Z3 + (b4 - b4_)*Z4 + (b5 - b5_)*Z5 + (b6 - b6_)*Z6);


    // Error estimation and Calculation of optimal tau
    rvecd scale_vec = rvecd(atol) + max(abs(y0), abs(y1))*rvecd(rtol);
    real err = scaled_norm(err_vec, scale_vec);
    tau = tau * min(tau_fac_max, max(tau_fac_min, tau_fac*pow(1.0 / max(err, 1e-10), 1.0/5.0))); // max(err, 1e-10) to prevent division by zero // 5.0 = order of rk5

    if (err >= 1.0) {
        isAccepted = false;
        return y0;
    }

    isAccepted = true;
    return y1;
}

rvecd rk45(rvecd y0, real t0, real t_end, out uint error_code, out uint step_counter, out uint same_step_counter) {
    // Guess initial step size (according to ChatGPT)
    rvecd z0 = rhs(y0);
    real y0_norm = scaled_norm(y0, rvecd(atol) + abs(y0)*rvecd(rtol));
    real z0_norm = scaled_norm(z0, rvecd(atol) + abs(z0)*rvecd(rtol));
    real tau = 0.01 * y0_norm / max(z0_norm, 1e-10); // prevent divide by 0.0
    tau = min(1.0, max(1e-8, tau)); // clamp to range [1e-8, 1.0]

    step_counter = 0;
    same_step_counter = 0;
    real t = t0;
    rvecd y = y0;
    while(t < t_end - 1e-9) { // small tolerance to rounding errors
        if (step_counter >= MAX_STEPS) {
            error_code = ERR_TOO_MANY_STEPS;
            return rvecd(0.0);
        }
        if (same_step_counter >= MAX_SAME_STEPS) {
            error_code = ERR_TOO_MANY_SAME_STEPS;
            return rvecd(0.0);
        }
        if (tau < MIN_TAU && t + tau < t_end - 1e-9) { // tau too small and not close to end
            error_code = ERR_TAU_TOO_SMALL;
            return rvecd(0.0);
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
#endif //  #ifndef RK45_DISABLE_VEC_METHODS


// * _arr methods (taking an M-element-array of D-dimensional vectors)
#ifndef RK45_DISABLE_ARR_METHODS

// Same as above but for arrays of vectors
// I believe GLSL compiler will unroll loops and nested loops, since M is known at compile time
real scaled_norm_arr(rvecd vector[M], rvecd scale_vec[M]) {
    real sum_of_squares = 0.0;
    for (int i = 0; i < M; ++i) {
        rvecd temp = vector[i] / scale_vec[i];
        sum_of_squares += dot(temp, temp);
    }
    return sqrt(sum_of_squares / (M*D)); // 2-norm
}

void rk45_step_arr(rvecd y0[M], inout real tau, out bool isAccepted, out rvecd y1[M]) {
    rvecd Y1[M];
    rvecd Z1[M];
    rvecd Y2[M];
    rvecd Z2[M];
    rvecd Y3[M];
    rvecd Z3[M];
    rvecd Y4[M];
    rvecd Z4[M];
    rvecd Y5[M];
    rvecd Z5[M];
    rvecd Y6[M];
    rvecd Z6[M];
    rvecd err_vec[M];

    for (int i = 0; i < M; ++i) {
        Y1[i] = y0[i];
    }
    rhs_arr(Y1, Z1); // set Z1

    for (int i = 0; i < M; ++i) {
        Y2[i] = y0[i] + tau*(a21*Z1[i]);
    }
    rhs_arr(Y2, Z2); // set Z2

    for (int i = 0; i < M; ++i) {
        Y3[i] = y0[i] + tau*(a31*Z1[i] + a32*Z2[i]);
    }
    rhs_arr(Y3, Z3); // set Z3

    for (int i = 0; i < M; ++i) {
        Y4[i] = y0[i] + tau*(a41*Z1[i] + a42*Z2[i] + a43*Z3[i]);
    }
    rhs_arr(Y4, Z4); // set Z4

    for (int i = 0; i < M; ++i) {
        Y5[i] = y0[i] + tau*(a51*Z1[i] + a52*Z2[i] + a53*Z3[i] + a54*Z4[i]);
        
    }
    rhs_arr(Y5, Z5); // set Z5

    for (int i = 0; i < M; ++i) {
        Y6[i] = y0[i] + tau*(a61*Z1[i] + a62*Z2[i] + a63*Z3[i] + a64*Z4[i] + a65*Z5[i]);
    }
    rhs_arr(Y6, Z6); // set Z6

    for (int i = 0; i < M; ++i) {
        y1[i] = y0[i] + tau*(b1*Z1[i] + b2*Z2[i] + b3*Z3[i] + b4*Z4[i] + b5*Z5[i] + b6*Z6[i]);
        err_vec[i] = tau*((b1 - b1_)*Z1[i] + (b2 - b2_)*Z2[i] + (b3 - b3_)*Z3[i] + (b4 - b4_)*Z4[i] + (b5 - b5_)*Z5[i] + (b6 - b6_)*Z6[i]);
    }


    // Error estimation and Calculation of optimal tau
    rvecd scale_vec[M];
    for (int i = 0; i < M; ++i) {
        scale_vec[i] = rvecd(atol) + max(abs(y0[i]), abs(y1[i]))*rvecd(rtol);
    }
    real err = scaled_norm_arr(err_vec, scale_vec);
    tau = tau * min(tau_fac_max, max(tau_fac_min, tau_fac*pow(1.0 / max(err, 1e-10), 1.0/5.0))); // max(err, 1e-10) to prevent division by zero // 5.0 = order of rk5

    if (err >= 1.0) {
        isAccepted = false;
        for (int i = 0; i < M; ++i) {
            y1[i] = y0[i]; // output y0
            return;
        }
    }

    isAccepted = true;
    // y1 is already set
}

void rk45_arr(rvecd y0[M], real t0, real t_end, out uint error_code, out uint step_counter, out uint same_step_counter, out rvecd y[M]) {
    // Guess initial step size (according to ChatGPT)
    rvecd z0[M];
    rvecd scale_vec_y0[M];
    rvecd scale_vec_z0[M];

    rhs_arr(y0, z0); // set z0
    for (int i = 0; i < M; ++i) {
        scale_vec_y0[i] = rvecd(atol) + abs(y0[i])*rvecd(rtol);
        scale_vec_z0[i] = rvecd(atol) + abs(z0[i])*rvecd(rtol);
    }
    real y0_norm = scaled_norm_arr(y0, scale_vec_y0);
    real z0_norm = scaled_norm_arr(z0, scale_vec_z0);
    real tau = 0.01 * y0_norm / max(z0_norm, 1e-10); // prevent divide by 0.0
    tau = min(1.0, max(1e-8, tau)); // clamp to range [1e-8, 1.0]

    step_counter = 0;
    same_step_counter = 0;
    real t = t0;
    for (int i = 0; i < M; ++i) {
        y[i] = y0[i];
    }
    while(t < t_end - 1e-9) { // small tolerance to rounding errors
        if (step_counter >= MAX_STEPS) {
            error_code = ERR_TOO_MANY_STEPS;
            for (int i = 0; i < M; ++i) {
                y[i] = rvecd(0.0);
            }
            return;
        }
        if (same_step_counter >= MAX_SAME_STEPS) {
            error_code = ERR_TOO_MANY_SAME_STEPS;
            for (int i = 0; i < M; ++i) {
                y[i] = rvecd(0.0);
            }
            return;
        }
        if (tau < MIN_TAU && t + tau < t_end - 1e-9) { // tau too small and not close to end
            error_code = ERR_TAU_TOO_SMALL;
            for (int i = 0; i < M; ++i) {
                y[i] = rvecd(0.0);
            }
            return;
        }

        bool isAccepted;
        tau = min(tau, t_end - t); // make sure not to overshoot t_end
        real used_tau = tau;
        rk45_step_arr(y, tau, isAccepted, y); // tau is always updated to new step size. On success next y is updated, otherwise y stays untouched

        if (isAccepted) {
            same_step_counter = 0;
            t += used_tau;
        } else {
            same_step_counter += 1;
        }

        step_counter += 1;
    }

    error_code = SUCCESS;
    // y is already set
}
#endif //  #ifndef RK45_DISABLE_ARR_METHODS

#endif
