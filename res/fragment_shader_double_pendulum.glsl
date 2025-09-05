#version 430 core

uniform uvec2 windowSize;
uniform double zoomScale;
uniform dvec2 numberStart;
uniform uint maxIterations;
uniform float t_end;
uniform float v1_start;
uniform float v2_start;

#include "real.glsl"
#include "colormaps.glsl"
#include "double_pendulum_rhs.glsl"
#include "rk45.glsl"

// uniform uint colorAccuracy = 10; // used by some flowColor methods: 1 means every index results in a colorStep of (1.0 / 1) but there are only (6 * 1) colors. 255 means, that every index results in a much smaller colorStep of (1.0 / 255) but there are (255 * 6) colors. 

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


// vec3 HSVToRGB(float h, float s, float v) {
//     vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
//     vec3 p = abs(fract(vec3(h, h, h) + K.xyz) * 6.0 - K.www);
//     return v * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), s);
// }

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
		v1_start, // 0.5,
		v2_start  // -0.5
	);

	// This one is drippy
	// rvec4 y_start = rvec4(
	// 	q1_start * 1.0,
	// 	q1_start * 1.0,
	// 	q2_start * v1_start,
	// 	q2_start * v2_start
	// );

	// rvec4 y_start = rvec4(
	// 	q1_start * 1.0,
	// 	q2_start * 1.0,
	// 	q2_start * v1_start,
	// 	q2_start * v2_start
	// );

	uint status;
	uint step_counter;
	uint same_step_counter;
	rvec4 y = rk45(y_start, status, step_counter, same_step_counter);
	if (status == ERR_TOO_MANY_STEPS) {
		fragColor = vec4(1.0, 0.7, 0.0, 1.0); // orange
		return;
	} else if (status == ERR_TOO_MANY_SAME_STEPS) { // did not reach end after maxIterations
		fragColor = vec4(1.0, 0.0, 0.7, 1.0); // purple
		return;
	} else if (status == ERR_TAU_TOO_SMALL) {
		fragColor = vec4(1.0, 1.0, 1.0, 1.0); // white
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
	// float value = remap(float(y1), -1.0, 1.0, 0.0, 1.0);
	// float value = remap(float(y1 + y2), -3.0, 3.0, 0.0, 1.0);
	// float value = remap(float(x2), -2.0, 2.0, 0.0, 1.0);
	// float value = remap(float(q1), -2*3.41, 2*3.41, 0.0, 1.0); // for angles need cyclic colormap
	// float value = remap(float(length(rvec2(x1, x2))), 0.0, sqrt(3), 0.0, 1.0);
	// float value = remap(float(length(rvec2(x2, y2))), 0.0, 4.0, 0.0, 1.0);


	fragColor = plasma(value);
}
