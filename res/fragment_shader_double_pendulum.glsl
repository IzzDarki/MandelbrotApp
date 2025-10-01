#version 430 core

uniform uvec2 windowSize;
uniform double zoomScale;
uniform dvec2 numberStart;
uniform uvec2 tileOffset; // used for tiled screenshot rendering

uniform float v1_start;
uniform float v2_start;

#include "real.glsl"
#include "colormaps.glsl"
#include "double_pendulum_rhs.glsl"
#define RK45_DISABLE_ARR_METHODS // We only need vector methods (since 4 dimensions fit in one vector)
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
	// Without tiled rendering
	// real q1_start = real(zoomScale * double(gl_FragCoord.x) / windowSize.x + numberStart.x);
	// real q2_start = real((zoomScale * double(gl_FragCoord.y) + numberStart.y * windowSize.y) / windowSize.x);

	// With tiled rendering (old)
	// rvec2 globalPixel = rvec2(real(tileOffset.x), real(tileOffset.y)) + rvec2(gl_FragCoord.xy);
	// real q1_start = real(zoomScale) * globalPixel.x / real(windowSize.x) + real(numberStart.x);
	// real q2_start = (real(zoomScale) * globalPixel.y + real(numberStart.y) * real(windowSize.y)) / real(windowSize.x);

	// With tiled rendering
	// (Compute start value in double precision, then round to real -> might lead to a little increase in maximum zoom depth)
	real q1_start = real(zoomScale * double(gl_FragCoord.x + tileOffset.x) / windowSize.x + numberStart.x);
	real q2_start = real((zoomScale * double(gl_FragCoord.y + tileOffset.y) + numberStart.y * windowSize.y) / windowSize.x);



	rvec4 y_start = rvec4(
		q1_start,
		q2_start,
		v1_start, // 0.5,
		v2_start  // -0.5
	);

	// This one is drippy (especially when playing with parameters)
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
	rvec4 y = rk45(y_start, t0, t_end, status, step_counter, same_step_counter);
	if (status == ERR_TOO_MANY_STEPS) {
		fragColor = vec4(1.0, 0.7, 0.0, 1.0); // orange
		return;
	} else if (status == ERR_TOO_MANY_SAME_STEPS) { // did not reach end after MAX_STEPS
		fragColor = vec4(1.0, 0.0, 0.7, 1.0); // purple
		return;
	} else if (status == ERR_TAU_TOO_SMALL) {
		fragColor = vec4(1.0, 0.7, 0.7, 1.0); // light red
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

	float value = remap(float(y2), -(l1+l2), l1+l2, 0.0, 1.0);
	// float value = remap(float(y1), -1.0, 1.0, 0.0, 1.0);
	// float value = remap(float(y1 + y2), -3.0, 3.0, 0.0, 1.0);
	// float value = remap(float(x2), -2.0, 2.0, 0.0, 1.0);
	// float value = remap(float(q1), -2*3.41, 2*3.41, 0.0, 1.0); // for angles need cyclic colormap
	// float value = remap(float(length(rvec2(x1, x2))), 0.0, sqrt(3), 0.0, 1.0);
	// float value = remap(float(length(rvec2(x2, y2))), 0.0, 4.0, 0.0, 1.0);
	// float value = remap(float(step_counter), 0.0, float(MAX_STEPS / 3.0), 0.0, 1.0);

	fragColor = inferno(value);
}
