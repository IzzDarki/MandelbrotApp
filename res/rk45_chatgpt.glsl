

// --- helper: elementwise absolute and max ---
rvec4 rabs(rvec4 x) { return rvec4(abs(x.x), abs(x.y), abs(x.z), abs(x.w)); }
rvec4 rmaxv(rvec4 a, rvec4 b) { return rvec4(max(a.x,b.x), max(a.y,b.y), max(a.z,b.z), max(a.w,b.w)); }

// --- compute a scaled infinity norm of the error vector (for adaptivity) ---
// err = max_i ( |e_i| / (abstol + reltol * max(|y_i|, |ynew_i|)) )
real rk45_error_norm(rvec4 e, rvec4 y, rvec4 ynew, real abstol, real reltol)
{
    rvec4 denom = rmaxv(rabs(y), rabs(ynew));
    denom = rvec4(abstol, abstol, abstol, abstol) + denom * rvec4(reltol, reltol, reltol, reltol);
    rvec4 ratio = rabs(e) / denom;
    // infinity norm (max component)
    real m = max(max(ratio.x, ratio.y), max(ratio.z, ratio.w));
    return m;
}

/*
  Adaptive RK45 integrator (Cash-Karp coefficients).
  Integrates state y from time t to t_end (forward or backward) with adaptive step.
  - y: in/out state vector (rvec4)
  - t: in/out current time (real). On return contains reached time (t_end if success)
  - h_init: initial step size guess (if <= 0, a heuristic is used)
  - abstol, reltol: tolerances (typical e.g. 1e-6)
  - max_steps: global cap of accepted steps (safety)
  - max_tries_per_step: how many shrink attempts before failing (safety)
  - out status (int):
    0 = success (reached t_end)
    1 = failed: step size underflow / couldn't meet tolerance
    2 = failed: exceeded max_steps
  Returns:
	y is returned as an numerical approximation to y(t_end)
*/
rvec4 solve_rk45(
    inout rvec4 y,
    inout real t,
    in real h_init,
    in real abstol,
    in real reltol,
    in uint max_steps,
    in uint max_tries_per_step,
	out int status
) {
    // Cash-Karp coefficients (as reals)
    const real a2 = 1.0/5.0;
    const real a3 = 3.0/10.0;
    const real a4 = 3.0/5.0;
    const real a5 = 1.0;
    const real a6 = 7.0/8.0;

    const real b21 = 1.0/5.0;

    const real b31 = 3.0/40.0;     const real b32 = 9.0/40.0;

    const real b41 = 3.0/10.0;     const real b42 = -9.0/10.0;  const real b43 = 6.0/5.0;

    const real b51 = -11.0/54.0;   const real b52 = 5.0/2.0;    const real b53 = -70.0/27.0; const real b54 = 35.0/27.0;

    const real b61 = 1631.0/55296.0; const real b62 = 175.0/512.0; const real b63 = 575.0/13824.0;
    const real b64 = 44275.0/110592.0; const real b65 = 253.0/4096.0;

    // 5th-order solution weights
    const real c1 = 37.0/378.0;
    const real c3 = 250.0/621.0;
    const real c4 = 125.0/594.0;
    const real c6 = 512.0/1771.0;

    // 4th-order solution weights (for error estimate)
    const real cs1 = 2825.0/27648.0;
    const real cs3 = 18575.0/48384.0;
    const real cs4 = 13525.0/55296.0;
    const real cs5 = 277.0/14336.0;
    const real cs6 = 1.0/4.0;

    // Safety limits for step adaptation
    const real SAFETY = 0.84;
    const real MIN_SHRINK = 0.1;
    const real MAX_GROW = 4.0;

    // Step-size heuristic and bounds
    real h = h_init;
    real dt = t_end - t;
    if (h <= real(0.0)) {
        h = dt * 0.01; // small guess
        if (h == real(0.0)) h = real(1e-6);
    }

    // Ensure h has same sign as dt (allow backward integration)
    if (dt < real(0.0) && h > real(0.0)) h = -h;
    if (dt > real(0.0) && h < real(0.0)) h = -h;

    const real H_MIN = abs(dt) * real(1e-12); // absolute minimum step allowed relative to interval
    const real H_MAX = abs(dt) * real(1.0);   // don't take steps larger than total interval

    int steps = 0;

    // temporaries
    rvec4 k1, k2, k3, k4, k5, k6;
    rvec4 yt, y5, y4, e;

    // bounded main loop: accept up to max_steps steps
    for (int step = 0; step < max_steps; ++step) {
        // If close to target, clamp one final step to hit t_end
        dt = t_end - t;
        if (abs(dt) <= real(0.0)) {
			status = 0;
            return y; // already at target
        }
        if (abs(h) > abs(dt)) {
            h = dt;
        }
        // Inner try loop to adjust step until tolerance satisfied (bounded)
        bool accepted = false;
        for (int tries = 0; tries < max_tries_per_step; ++tries) {
            // compute stages
            k1 = rhs(y);                      // k1 = f(y)
            yt = y + rvec4(h * b21) * k1;
            k2 = rhs(yt);
            yt = y + rvec4(h * b31) * k1 + rvec4(h * b32) * k2;
            k3 = rhs(yt);
            yt = y + rvec4(h * b41) * k1 + rvec4(h * b42) * k2 + rvec4(h * b43) * k3;
            k4 = rhs(yt);
            yt = y + rvec4(h * b51) * k1 + rvec4(h * b52) * k2 + rvec4(h * b53) * k3 + rvec4(h * b54) * k4;
            k5 = rhs(yt);
            yt = y + rvec4(h * b61) * k1 + rvec4(h * b62) * k2 + rvec4(h * b63) * k3 + rvec4(h * b64) * k4 + rvec4(h * b65) * k5;
            k6 = rhs(yt);

            // 5th order solution
            y5 = y + h * (rvec4(c1) * k1 + rvec4(c3) * k3 + rvec4(c4) * k4 + rvec4(c6) * k6);

            // 4th order solution (for error estimate)
            y4 = y + h * (rvec4(cs1) * k1 + rvec4(cs3) * k3 + rvec4(cs4) * k4 + rvec4(cs5) * k5 + rvec4(cs6) * k6);

            // error estimate vector and norm
            e = y5 - y4;
            real err = rk45_error_norm(e, y, y5, abstol, reltol);

            // Accept if error small enough (err <= 1)
            if (err <= real(1.0)) {
                // accept step
                t = t + h;
                y = y5;
                steps++;
                accepted = true;

                // compute suggested new h
                // avoid divide by zero
                real factor = SAFETY;
                if (err > real(0.0)) {
                    factor = SAFETY * pow(err, real(-0.2)); // exponent -1/(order+1) where order=4 -> -1/5 ~ -0.2
                }
                factor = clamp(factor, MIN_SHRINK, MAX_GROW);
                h = h * factor;
                // clamp step to not overshoot and to sane bounds
                if (abs(h) > H_MAX) h = (h > 0.0 ? H_MAX : -H_MAX);
                if (abs(h) < H_MIN) h = (h > 0.0 ? H_MIN : -H_MIN);
                break;
            } else {
                // too large error: reduce step and retry
                real factor = SAFETY * pow(err, real(-0.2));
                factor = max(factor, MIN_SHRINK);
                factor = min(factor, real(0.9)); // reduce, but don't invert sign; cap to 0.9 to avoid aggressive tiny steps
                h = h * factor;
                if (abs(h) < H_MIN) {
                    // step-size underflow -> fail
					status = 1;
                    return y;
                }
                // continue tries
            }
        } // end tries loop

        if (!accepted) {
            // couldn't satisfy tolerance within tries
			status = 1;
            return y;
        }

        // If we've reached or passed t_end (within machine tolerance), we're done
        if ( (t_end - t) == real(0.0) || sign(t_end - t) != sign(t_end - (t + h)) ) {
            // exact finish or sign would change; but safer: check closeness
        }
        // If we are at target (within machine epsilon)
        if (abs(t_end - t) <= real(1e-12) * max(abs(t), abs(t_end))) {
            t = t_end;
            status = 0;
			return y;
        }
    } // end step loop

    // too many steps
	status = 2;
    return y;
}
