#version 430 core

#include "complex.glsl"
#include "zooming_and_tiling.glsl"

uniform uint maxIterations = 400;
uniform float colorScale = 50.0; // E.g. 50.0 means that the value range [0, 50] contains the entire colormap. Cyclic colormaps repeat, others are clamped. 
uniform uint sliceValue = 0;
uniform float sliceFactor = 0.5;

uniform sampler2D colormap;

out vec4 fragColor;

uint calcFractal(complex start, out complex escape) {
	complex current = start;
	for (uint n = 1u; n < maxIterations + 1u; n++) {
		if (dot(current, current) > 65536.0) { // Divergence check // 4.0 would be enough, but higher values improve the smoothing
			escape = current;
			return n;
		}
		current = cmul(current, current) + start;
	}

	escape = current;
	return 0;
}

// #if FLOW_COLOR_TYPE == 0



// #elif FLOW_COLOR_TYPE == 1

// vec4 flowColor(uint index) {
// 	if (sliceFactor*sliceValue <= index && index <= sliceValue)
// 		return vec4(0.0, 0.0, 0.0, 1.0);
// 	else
// 		return vec4(1.0, 1.0, 1.0, 1.0);
// }

// #elif FLOW_COLOR_TYPE == 2


// #elif FLOW_COLOR_TYPE == 3


// vec4 flowColor(uint index, double escapeReal, double escapeImag) {
// 	if (index == 0) {
// 		return vec4(0.0f, 0.0f, 0.0f, 1.0f);
// 	}

// 	float smoothStep = float(index) + 1.0f - 0.42 * log(log(sqrt(float(escapeReal * escapeReal + escapeImag * escapeImag)))) / 0.301029996f;

// 	return rgbStep(smoothStep);

// 	// if (hue < 0.0f)
// 	// 	return vec4(1.0, 0, 0, 1.0);
// 	// else if (hue > 1.0f)
// 	// 	return vec4(0.0, 0.0, 1.0, 1.0);
// 	// while (hue > 360.0)
// 	// 	hue -= 360.0;
//     // while (hue < 0.0)
// 	// 	hue += 360.0;

// 	// float r = hue;
// 	// float g = r * r;
// 	// float b = r * g;
// 	// return vec4(r, g, b, 1.0);

// 	// vec3 color = HSVToRGB(hue, 0.8f, 1.0f);
// 	// return vec4(color.x, color.y, color.z, 1.0);
// }

// #endif

void main() {
	// double real = zoomScale * (double(gl_FragCoord.x) + 0.5) / windowSize.x + numberStart.x;
	// double imag = (zoomScale * (double(gl_FragCoord.y) + 0.5) + numberStart.y * windowSize.y) / windowSize.x;

	dvec2 pixelCoord = dvec2(gl_FragCoord.xy); // gl_FragCoord (vec4) gives the fragments center position in window coordinates, e.g. the lower left is vec4(0.5, 0.5, _, _)
	complex start = complex(pixelCoordToPlaneCoord(pixelCoord));

	complex escape;
	uint count = calcFractal(start, escape);

#ifdef USE_SMOOTHING
	float smoothCount = float(count) + 1.0 - log2(log(float(max(length(escape), 2.0)))); // Specifically for Mandelbrot set
#else
	float smoothCount = float(count);
#endif

	if (count == 0) {
		fragColor = vec4(0.0, 0.0, 0.0, 1.0); // Mandelbrot itself black
	} else {
		smoothCount = smoothCount / colorScale;
		fragColor = texture(colormap, vec2(smoothCount, 0.5));
	}
}
