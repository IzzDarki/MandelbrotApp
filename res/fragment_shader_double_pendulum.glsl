#version 430 core

#include "colormaps.glsl"

uniform uvec2 windowSize;
uniform double zoomScale;
uniform dvec2 numberStart;
uniform uint maxIterations;
uniform float end_time;

// uniform uint colorAccuracy = 10; // used by some flowColor methods: 1 means every index results in a colorStep of (1.0 / 1) but there are only (6 * 1) colors. 255 means, that every index results in a much smaller colorStep of (1.0 / 255) but there are (255 * 6) colors. 

// Double vs float
// #define USE_DOUBLE // Double is almost unusable, since GLSL does not provide sine and cosine for double precision
#ifdef USE_DOUBLE
	#define real double
	#define rvec2 dvec2
	#define rmat2 dmat2
	#define rvec3 dvec3
	#define rmat3 dmat3
	#define rvec4 dvec4
	#define rmat4 dmat4
	#define rsin(x) sina_9((x))
	#define rcos(x) cosa_9((x))
	#define rpow(x, exp) (x)*(x) // double(pow(float((x)), (exp))) // ! This is a hack

	//sin approximation, error < 5e-9
	double sina_9(double x)
	{
		//minimax coefs for sin for 0..pi/2 range
		const double a3 = -1.666665709650470145824129400050267289858e-1LF;
		const double a5 =  8.333017291562218127986291618761571373087e-3LF;
		const double a7 = -1.980661520135080504411629636078917643846e-4LF;
		const double a9 =  2.600054767890361277123254766503271638682e-6LF;

		const double m_2_pi = 0.636619772367581343076LF;
		const double m_pi_2 = 1.57079632679489661923LF;

		double y = abs(x * m_2_pi);
		double q = floor(y);
		int quadrant = int(q);

		double t = (quadrant & 1) != 0 ? 1 - y + q : y - q;
		t *= m_pi_2;

		double t2 = t * t;
		double r = fma(fma(fma(fma(a9, t2, a7), t2, a5), t2, a3), t2*t, t);

		r = x < 0 ? -r : r;

		return (quadrant & 2) != 0 ? -r : r;
	}

	//sin approximation, error < 2e-11
	double sina_11(double x)
	{
		//minimax coefs for sin for 0..pi/2 range
		const double a3 = -1.666666660646699151540776973346659104119e-1LF;
		const double a5 =  8.333330495671426021718370503012583606364e-3LF;
		const double a7 = -1.984080403919620610590106573736892971297e-4LF;
		const double a9 =  2.752261885409148183683678902130857814965e-6LF;
		const double ab = -2.384669400943475552559273983214582409441e-8LF;

		const double m_2_pi = 0.636619772367581343076LF;
		const double m_pi_2 = 1.57079632679489661923LF;

		double y = abs(x * m_2_pi);
		double q = floor(y);
		int quadrant = int(q);

		double t = (quadrant & 1) != 0 ? 1 - y + q : y - q;
		t *= m_pi_2;

		double t2 = t * t;
		double r = fma(fma(fma(fma(fma(ab, t2, a9), t2, a7), t2, a5), t2, a3),
			t2*t, t);

		r = x < 0 ? -r : r;

		return (quadrant & 2) != 0 ? -r : r;
	}

	//cos approximation, error < 5e-9
	double cosa_9(double x)
	{
		//sin(x + PI/2) = cos(x)
		return sina_9(x + 1.57079632679489661923LF);
	}

	//cos approximation, error < 2e-11
	double cosa_11(double x)
	{
		//sin(x + PI/2) = cos(x)
		return sina_11(x + 1.57079632679489661923LF);
	}
#else
	precision highp float;
	#define real float
	#define rvec2 vec2
	#define rmat2 mat2
	#define rvec3 vec3
	#define rmat3 mat3
	#define rvec4 vec4
	#define rmat4 mat4
	#define rsin(x) sin((x))
	#define rcos(x) cos((x))
	#define rpow(x, y) pow((x), (y))
#endif

// Double Pendulum Parameters
#define g 9.81
#define l1 1.0
#define l2 1.0
#define m1 1.0
#define m2 1.0
#define p1_start 0.0
#define p2_start 0.0

out vec4 fragColor;

void dH(rvec2 q, rvec2 p, out rvec2 dH_dq, out rvec2 dH_dp) {

	dH_dq = rvec2(
		-g*l1*(-m1 - m2)*rsin(q[0])
		+ p[0]*(-rpow(l1, 2)*m2*p[0]*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q[0] - q[1]), 2) + rpow(l1, 2)*m2, 2) + l1*l2*m2*p[1]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) + (1.0/2.0)*p[1]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2))
		+ p[1]*(l1*l2*m2*p[0]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) - rpow(l2, 2)*rpow(m2, 2)*p[1]*(m1 + m2)*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q[0] - q[1]), 2) + rpow(l2, 2)*rpow(m2, 2), 2) + (1.0/2.0)*p[0]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2)),

		g*l2*m2*rsin(q[1])
		+ p[0]*(rpow(l1, 2)*m2*p[0]*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q[0] - q[1]), 2) + rpow(l1, 2)*m2, 2) - l1*l2*m2*p[1]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) - 1.0/2.0*p[1]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2))
		+ p[1]*(-l1*l2*m2*p[0]*rsin(q[0] - q[1])*rpow(rcos(q[0] - q[1]), 2)/rpow(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2, 2) + rpow(l2, 2)*rpow(m2, 2)*p[1]*(m1 + m2)*rsin(q[0] - q[1])*rcos(q[0] - q[1])/rpow(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q[0] - q[1]), 2) + rpow(l2, 2)*rpow(m2, 2), 2) - 1.0/2.0*p[0]*rsin(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2))
	);

	dH_dp = rvec2(
		p[0]/(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q[0] - q[1]), 2) + rpow(l1, 2)*m2) - p[1]*rcos(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2),
		-p[0]*rcos(q[0] - q[1])/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q[0] - q[1]), 2) + l1*l2*m2) + p[1]*(m1 + m2)/(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q[0] - q[1]), 2) + rpow(l2, 2)*rpow(m2, 2))
	);

	return;
}

rvec4 rhs(rvec4 y) {
	real q1 = y[0];
	real q2 = y[1];
	real v1 = y[2];
	real v2 = y[3];

	return rvec4(
		v1,
		v2,
		(-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))/(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q1 - q2), 2) + rpow(l1, 2)*m2) - (-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2),
		(m1 + m2)*(-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))/(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q1 - q2), 2) + rpow(l2, 2)*rpow(m2, 2)) - (-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2)
	);
}

rvec2 rhs_1(rvec4 y) {
	return y.zw;
}

rvec2 rhs_2(rvec4 y) {
	real q1 = y[0];
	real q2 = y[1];
	real v1 = y[2];
	real v2 = y[3];

	return rvec2(
		(-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))/(rpow(l1, 2)*m1 - rpow(l1, 2)*m2*rpow(rcos(q1 - q2), 2) + rpow(l1, 2)*m2) - (-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2),
		(m1 + m2)*(-g*l2*m2*rsin(q2) + l1*l2*m2*rpow(v1, 2)*rsin(q1 - q2))/(rpow(l2, 2)*m1*m2 - rpow(l2, 2)*rpow(m2, 2)*rpow(rcos(q1 - q2), 2) + rpow(l2, 2)*rpow(m2, 2)) - (-g*l1*(m1 + m2)*rsin(q1) - l1*l2*m2*rpow(v2, 2)*rsin(q1 - q2))*rcos(q1 - q2)/(l1*l2*m1 - l1*l2*m2*rpow(rcos(q1 - q2), 2) + l1*l2*m2)
	);
}

rvec4 solve_semi_explicit_euler(
	real q1_start, real q2_start
) {
	const real tau = real(end_time) / maxIterations;
	rvec4 y = rvec4(
		q1_start,
		q2_start,
		0.5,
		-0.5
	);

	for (int n = 0; n < maxIterations; ++n) {
		y.xy = y.xy + tau * rhs_1(y);
		y.zw = y.zw + tau * rhs_2(y);
	}

	return y;
}


rvec4 solve_explicit_euler(
	real q1_start, real q2_start
) {
	const real tau = real(end_time) / maxIterations;
	rvec4 y = rvec4(
		q1_start,
		q2_start,
		0.5,
		-0.5
	);

	for (int n = 0; n < maxIterations; ++n) {
		y = y + tau * rhs(y);
	}
	return y;
}

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
  Integrates state y from time t to end_time (forward or backward) with adaptive step.
  - y: in/out state vector (rvec4)
  - t: in/out current time (real). On return contains reached time (end_time if success)
  - h_init: initial step size guess (if <= 0, a heuristic is used)
  - abstol, reltol: tolerances (typical e.g. 1e-6)
  - max_steps: global cap of accepted steps (safety)
  - max_tries_per_step: how many shrink attempts before failing (safety)
  - out status (int):
    0 = success (reached end_time)
    1 = failed: step size underflow / couldn't meet tolerance
    2 = failed: exceeded max_steps
  Returns:
	y is returned as an numerical approximation to y(end_time)
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
    real dt = end_time - t;
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
        // If close to target, clamp one final step to hit end_time
        dt = end_time - t;
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

        // If we've reached or passed end_time (within machine tolerance), we're done
        if ( (end_time - t) == real(0.0) || sign(end_time - t) != sign(end_time - (t + h)) ) {
            // exact finish or sign would change; but safer: check closeness
        }
        // If we are at target (within machine epsilon)
        if (abs(end_time - t) <= real(1e-12) * max(abs(t), abs(end_time))) {
            t = end_time;
            status = 0;
			return y;
        }
    } // end step loop

    // too many steps
	status = 2;
    return y;
}


vec3 HSVToRGB(float h, float s, float v) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(h, h, h) + K.xyz) * 6.0 - K.www);
    return v * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), s);
}

// map interval (a, b) to (c, d)
float remap(float x, float a, float b, float c, float d) {
    return c + (x - a) * (d - c) / (b - a);
}

void main() {
	real q1_start = real(zoomScale * (double(gl_FragCoord.x) + 0.5) / windowSize.x + numberStart.x);
	real q2_start = real((zoomScale * (double(gl_FragCoord.y) + 0.5) + numberStart.y * windowSize.y) / windowSize.x);

	rvec4 y_start = rvec4(
		q1_start,
		q2_start,
		0.5,
		-0.5
	);

	real time = 0.0;
	int status = -2;
	rvec4 y = solve_rk45(y_start, time, -1.0, 1e-6, 1e-6, maxIterations, 20, status);
	if (status == 1) { // step size underflow
		fragColor = vec4(1.0, 0.0, 1.0, 1.0);
		return;
	} else if (status == 2) { // did not reach end after maxIterations
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);
		return;
	}

	real q1 = y[0];
	real q2 = y[1];
	real v1 = y[2];
	real v2 = y[3];

	real x1 = l1 * rsin(q1);
	real y1 = l1 * -rcos(q1);
	real x2 = x1 + l2 * rsin(q2);
	real y2 = y1 + l2 * -rcos(q2);

	float value = remap(float(y2), -2.0, 2.0, 0.0, 1.0);

	fragColor = viridis(value);
}
