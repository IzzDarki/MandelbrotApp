#version 430 core

uniform uvec2 windowSize;
uniform double zoomScale;
uniform dvec2 numberStart;

uniform float v1_start;
uniform float v2_start;

#include "real.glsl"
#include "colormaps.glsl"
#include "n_body_problem_rhs.glsl" // Also defines D and N
#define RK45_DISABLE_VEC_METHODS // we only need array methods (since a vector can be at most 4-dimensional)
#include "rk45.glsl"

out vec4 fragColor;

// rvec4 solve_semi_explicit_euler(
// 	real q1_start, real q2_start
// ) {
// 	const real tau = real(t_end) / maxIterations;
// 	rvec4 y = rvec4(
// 		q1_start,
// 		q2_start,
// 		0.5,
// 		-0.5
// 	);

// 	for (int n = 0; n < maxIterations; ++n) {
// 		y.xy = y.xy + tau * rhs_1(y);
// 		y.zw = y.zw + tau * rhs_2(y);
// 	}

// 	return y;
// }


// rvec4 solve_explicit_euler(
// 	real q1_start, real q2_start
// ) {
// 	const real tau = real(t_end) / maxIterations;
// 	rvec4 y = rvec4(
// 		q1_start,
// 		q2_start,
// 		0.5,
// 		-0.5
// 	);

// 	for (int n = 0; n < maxIterations; ++n) {
// 		y = y + tau * rhs(y);
// 	}
// 	return y;
// }


// map interval (a, b) to (c, d)
float remap(float x, float a, float b, float c, float d) {
    return c + (x - a) * (d - c) / (b - a);
}

void main() {
	real r = real(zoomScale * (double(gl_FragCoord.x) + 0.5) / windowSize.x + numberStart.x);
	real s = real((zoomScale * (double(gl_FragCoord.y) + 0.5) + numberStart.y * windowSize.y) / windowSize.x);

	// Map parameters r, s to an initial state describing N bodies in a D-dimensional space and their momenta
	rvecd y_start[M] = rvecd[M](
		rvecd(0.0, 0.0, 0.0),     // position q[0]
		rvecd(1.2, 0.0, 0.0),     // position q[1]
		rvecd(1.2, s*1.2, r*1.2), // position q[2]
		rvecd(0.0, 1.2, 0.0),     // momentum p[0]
		rvecd(0.0, -1.2, 0.0),    // momentum p[1]
		rvecd(0.0, 0.0, -0.6)    // momentum p[2]	
	);


	uint status;
	uint step_counter;
	uint same_step_counter;
	rvecd y[M];
	rk45_arr(y_start, t0, t_end, status, step_counter, same_step_counter, y);
	if (status == ERR_TOO_MANY_STEPS) {
		fragColor = vec4(1.0, 0.7, 0.0, 1.0); // orange
		return;
	} else if (status == ERR_TOO_MANY_SAME_STEPS) { // did not reach end after MAX_STEPS
		fragColor = vec4(1.0, 0.0, 0.7, 1.0); // purple
		return;
	} else if (status == ERR_TAU_TOO_SMALL) {
		fragColor = vec4(1.0, 1.0, 1.0, 1.0); // white
		return;
	}

	// Position of i-th body q[i] = y[i]
	// Momentum of i-th body p[i] = y[N+i]

	float value = remap(float(length(y[0] - y[1])), 0.0, 30.0, 0.0, 1.0);
	// float value = remap(float(y[0].x), -10.0, 10.0, 0.0, 1.0);
	// float value = remap(float(step_counter), 0.0, float(MAX_STEPS / 3.0), 0.0, 1.0);


	fragColor = viridis(value);
}
