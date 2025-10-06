#version 430 core

uniform uvec2 windowSize;
uniform double zoomScale;
uniform dvec2 center;
uniform uvec2 tileOffset; // used for tiled screenshot rendering
double windowSizeMeasure = double(min(windowSize.x, windowSize.y));

uniform float v1_start;
uniform float v2_start;

#include "real.glsl"
#include "colormaps.glsl"
#include "double_pendulum_rhs.glsl"

#define RK45_DISABLE_ARR_METHODS // We only need vector methods (since 4 dimensions fit in one vector)
#include "rk45.glsl"


#if SUPER_SAMPLING != 0

// Static super sampling
#include "static_supersampling.glsl"

#else

// Adaptive super sampling
real evaluate(dvec2 pixelCoord) {
	real q1_start = real((zoomScale / windowSizeMeasure) * (pixelCoord.x - double(windowSize.x) / 2.0) + center.x);
	real q2_start = real((zoomScale / windowSizeMeasure) * (pixelCoord.y - double(windowSize.y) / 2.0) + center.y);

	rvec4 y_start = rvec4(
		q1_start,
		q2_start,
		v1_start, // 0.5,
		v2_start  // -0.5
	);

	uint status;
	uint step_counter;
	uint same_step_counter;
	rvec4 y = rk45(y_start, t0, t_end, status, step_counter, same_step_counter);
	// status checks are useless here, since one cannot abort the fragment shader with an "error color" anyway

	real q1 = y[0];
	real q2 = y[1];
	real v1 = y[2];
	real v2 = y[3];

	real x1 = l1 * rsin(q1);
	real y1 = l1 * -rcos(q1);
	real x2 = x1 + l2 * rsin(q2);
	real y2 = y1 + l2 * -rcos(q2);
	return y2;
}
#include "adaptive_supersampling.glsl"

#endif


// map interval (a, b) to (c, d)
float remap(float x, float a, float b, float c, float d) {
    return c + (x - a) * (d - c) / (b - a);
}


out vec4 fragColor;
void main() {
	// Adaptive super sampling
	#if SUPER_SAMPLING == 0
	real result = evaluateWithAdaptiveSuperSampling(gl_FragCoord.xy + vec2(tileOffset));

	#else

	// Static super sampling
	dvec2 pixelCoord = dvec2(gl_FragCoord.xy) + dvec2(tileOffset); // gl_FragCoord (vec4) gives the fragments center position in window coordinates, e.g. the lower left is vec4(0.5, 0.5, _, _)
	real result = 0.0;
	for (uint i = 0; i < NUM_SAMPLES; ++i) {
		real q1_start = real((zoomScale / windowSizeMeasure) * (pixelCoord.x + sample_offsets[i].x - double(windowSize.x) / 2.0) + center.x);
		real q2_start = real((zoomScale / windowSizeMeasure) * (pixelCoord.y + sample_offsets[i].y - double(windowSize.y) / 2.0) + center.y);


		rvec4 y_start = rvec4(
			q1_start,
			q2_start,
			v1_start,
			v2_start
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
		result += y2;
	}
	result /= NUM_SAMPLES;

	#endif

	float value = remap(float(result), -(l1+l2), l1+l2, 0.0, 1.0);
	// float value = remap(float(y1), -1.0, 1.0, 0.0, 1.0);
	// float value = remap(float(y1 + y2), -3.0, 3.0, 0.0, 1.0);
	// float value = remap(float(x2), -2.0, 2.0, 0.0, 1.0);
	// float value = remap(float(q1), -2*3.41, 2*3.41, 0.0, 1.0); // for angles need cyclic colormap
	// float value = remap(float(length(rvec2(x1, x2))), 0.0, sqrt(3), 0.0, 1.0);
	// float value = remap(float(length(rvec2(x2, y2))), 0.0, 4.0, 0.0, 1.0);
	// float value = remap(float(step_counter), 0.0, float(MAX_STEPS / 3.0), 0.0, 1.0);

	fragColor = viridis(value);
}
