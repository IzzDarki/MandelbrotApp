#ifndef RK45_INCLUDED
#define RK45_INCLUDED

// Problem definition must contain this
// const float t0 = 0.0;
// const float t_end = 10.0;
// vec4 rhs(vec4 y0) {
//     return ...;
// }

// I trust the glsl compilation to optimize const float operations away
// https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta%E2%80%93Fehlberg_method, Formula 1
const float a21 = 2.0/9.0;
const float a31 = 1.0/12.0;       const float a32 = 1.0/4.0;
const float a41 = 69.0/128.0;     const float a42 = -243.0/128.0;   const float a43 = 135.0/64.0;
const float a51 = -17.0/12.0;     const float a52 = 27.0/4.0;       const float a53 = -27.0/5.0;    const float a54 = 16.0/15.0;
const float a61 = 65.0/432.0;     const float a62 = -5.0/16.0;      const float a63 = 13.0/16.0;    const float a64 = 4.0/27.0;     const float a65 = 5.0/144.0;

const float b1 = 47.0/450.0; const float b2 = 0.0;  const float b3 = 12.0/25.0; const float b4 = 32.0/225.0; const float b5 = 1.0/30.0; const float b6 = 6.0/25.0;
const float b1_ = 1.0/9.0;   const float b2_ = 0.0; const float b3_ = 9.0/12.0; const float b4_ = 16.0/45.0; const float b5_ = 1.0/12.0; const float b6_ = 0.0;

const float tau_fac_min = 1.0/3.0; const float tau_fac_max = 2.0; // tau_fac_max between 1.5 and 5.0 according to Numerik-Skript
const float tau_fac = 0.9; // tau_fac = 0.8 or 0.9 or pow(0.25, 1.0/p), where p = 5.0 in our case according to Numerik-Skript

const float rtol = 1e-10; // TODO should be uniform float
const float atol = 1e-10; // may not be 0.0 // TODO should be uniform float

const uint MAX_STEPS = 200000u;
const uint MAX_SAME_STEPS = 100u;

const uint SUCCESS = 0u;
const uint ERR_TOO_MANY_STEPS = 1u;
const uint ERR_TOO_MANY_SAME_STEPS = 2u;

float scaled_norm(vec4 vector, vec4 scale_vec) {
    vec4 temp = vector / scale_vec;
    return 0.5 * length(temp); // length is 2-norm
}

vec4 rk45_step(vec4 y0, inout float tau, out bool isAccepted) {
    vec4 Y1 = y0;
    vec4 Z1 = rhs(Y1);

    vec4 Y2 = y0 + tau*(a21*Z1);
    vec4 Z2 = rhs(Y2);

    vec4 Y3 = y0 + tau*(a31*Z1 + a32*Z2);
    vec4 Z3 = rhs(Y3);

    vec4 Y4 = y0 + tau*(a41*Z1 + a42*Z2 + a43*Z3);
    vec4 Z4 = rhs(Y4);

    vec4 Y5 = y0 + tau*(a51*Z1 + a52*Z2 + a53*Z3 + a54*Z4);
    vec4 Z5 = rhs(Y5);

    vec4 Y6 = y0 + tau*(a61*Z1 + a62*Z2 + a63*Z3 + a64*Z4 + a65*Z5);
    vec4 Z6 = rhs(Y6);

    vec4 y1 = y0 + tau*(b1*Z1 + b2*Z2 + b3*Z3 + b4*Z4 + b5*Z5 + b6*Z6);
    vec4 err_vec = tau*((b1 - b1_)*Z1 + (b2 - b2_)*Z2 + (b3 - b3_)*Z3 + (b4 - b4_)*Z4 + (b5 - b5_)*Z5 + (b6 - b6_)*Z6);


    // Error Estimation
    vec4 scale_vec = vec4(atol) + max(abs(y0), abs(y1))*vec4(rtol);
    float err = scaled_norm(err_vec, scale_vec);
    tau = tau * min(tau_fac_max, max(tau_fac_min, tau_fac*pow(1.0 / max(err, 1e-10), 1.0/5.0))); // max(err, 1e-10) to prevent division by zero

    if (err >= 1.0) {
        isAccepted = false;
        return y0;
    }

    isAccepted = true;
    return y1;
}

vec4 rk45(vec4 y0, out uint error_code, out uint step_counter, out uint same_step_counter) {
    // Guess initial step size (according to ChatGPT)
    vec4 z0 = rhs(y0);
    float y0_norm = scaled_norm(y0, vec4(atol) + abs(y0)*vec4(rtol));
    float z0_norm = scaled_norm(z0, vec4(atol) + abs(z0)*vec4(rtol));
    float tau = 0.01 * y0_norm / max(z0_norm, 1e-10); // prevent divide by 0.0
    tau = min(1.0, max(1e-8, tau)); // clamp to range [1e-8, 1.0]

    step_counter = 0;
    same_step_counter = 0;
    float t = t0;
    vec4 y = y0;
    while(t < t_end - 1e-9) { // small tolerance to rounding errors
        if (step_counter >= MAX_STEPS) {
            error_code = ERR_TOO_MANY_STEPS;
            return vec4(0.0);
        }
        if (same_step_counter >= MAX_SAME_STEPS) {
            error_code = ERR_TOO_MANY_SAME_STEPS;
            return vec4(0.0);
        }

        bool isAccepted;
        tau = min(tau, t_end - t); // make sure not to overshoot t_end
        float used_tau = tau;
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
