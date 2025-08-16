#version 430 core

uniform uvec2 windowSize;
uniform double zoomScale;
uniform dvec2 numberStart;
uniform uint maxIterations = 400;
uniform uint colorAccuracy = 10; // used by some flowColor methods: 1 means every index results in a colorStep of (1.0 / 1) but there are only (6 * 1) colors. 255 means, that every index results in a much smaller colorStep of (1.0 / 255) but there are (255 * 6) colors. 

out vec4 fragColor;

uint calcFractal(
	double startReal, double startImag
#if FLOW_COLOR_TYPE == 3
	,out double escapeReal, out double escapeImag
#endif
) {

	double real = 0;
	double imag = 0;
	double realTemp = 0;

	for (int n = 1; n < maxIterations + 1; n++) {
		if (CODE_DIVERGENCE_CRITERION) { // in case of mandelbrot (real * real) + (imag * imag) > 4
			return n;
		}
		realTemp = real;

		CODE_CALCULATE_NEXT_SEQUENCE_TERM // in case of mandelbrot real = (real * real) - (imag * imag) + startReal; imag = 2 * realTemp * imag + startImag;
	}
#if FLOW_COLOR_TYPE == 3
	escapeReal = real;
	escapeImag = imag;
#endif
	return 0;
}

#if FLOW_COLOR_TYPE == 0

vec4 flowColor(uint index) {
	if (index == 0)
		return vec4(0.0, 0.0, 0.0, 1.0);
	
	float r = 0.0;
	float g = 1.0;
	float b = 0.5333;

	float colorStep = float(index % (colorAccuracy * 6)) / (colorAccuracy);

	while (true) {
		if (r == 1.0 && g < 1.0 && b == 0.0) {
			if (g + colorStep > 1.0) {
				colorStep -= (1.0 - g);
				g = 1.0;
			}
			else {
				g += colorStep;
				break;
			}
		}
		else if (r > 0.0 && g == 1.0) {
			if (r - colorStep < 0.0) {
				colorStep -= r;
				r = 0.0;
			}
			else {
				r -= colorStep;
				break;
			}
		}
		else if (g == 1.0 && b < 1.0) {
			if (b + colorStep > 1.0) {
				colorStep -= (1.0 - b);
				b = 1.0;
			}
			else {
				b += colorStep;
				break;
			}
		}
		else if (g > 0.0 && b == 1.0) {
			if (g - colorStep < 0.0) {
				colorStep -= g;
				g = 0.0;
			}
			else {
				g -= colorStep;
				break;
			}
		}
		else if (b == 1.0 && r < 1.0) {
			if (r + colorStep > 1.0) {
				colorStep -= (1.0 - r);
				r = 1.0;
			}
			else {
				r += colorStep;
				break;
			}
		}
		else if (b > 0.0 && r == 1.0) {
			if (b - colorStep < 0.0) {
				colorStep -= b;
				b = 0.0;
			}
			else {
				b -= colorStep;
				break;
			}
		}
	}
	return vec4(r, g, b, 1.0);
}

#elif FLOW_COLOR_TYPE == 1

vec4 flowColor(uint index) {
	if (index != 0)
		return vec4(1.0, 1.0, 1.0, 1.0);
	else
		return vec4(0.0, 0.0, 0.0, 1.0);
}

#elif FLOW_COLOR_TYPE == 2

vec4 flowColor(uint index) {
	float brightness = 1.0 - 1.0 / exp(0.05 * float(index));
	
	float r = 0.2;
	float g = 0.0;
	float b = 1.0;

	const uint colorAccuracy = 500; // 1 means every index results in a colorStep of (1.0 / 1) but there are only (6 * 1) colors. 255 means, that every index results in a much smaller colorStep of (1.0 / 255) but there are (255 * 6) colors. 
	float colorStep = float(index % (colorAccuracy * 6)) / (colorAccuracy);

	while (true) {
		if (r == 1.0 && g < 1.0 && b == 0.0) {
			if (g + colorStep > 1.0) {
				colorStep -= (1.0 - g);
				g = 1.0;
			}
			else {
				g += colorStep;
				break;
			}
		}
		else if (r > 0.0 && g == 1.0) {
			if (r - colorStep < 0.0) {
				colorStep -= r;
				r = 0.0;
			}
			else {
				r -= colorStep;
				break;
			}
		}
		else if (g == 1.0 && b < 1.0) {
			if (b + colorStep > 1.0) {
				colorStep -= (1.0 - b);
				b = 1.0;
			}
			else {
				b += colorStep;
				break;
			}
		}
		else if (g > 0.0 && b == 1.0) {
			if (g - colorStep < 0.0) {
				colorStep -= g;
				g = 0.0;
			}
			else {
				g -= colorStep;
				break;
			}
		}
		else if (b == 1.0 && r < 1.0) {
			if (r + colorStep > 1.0) {
				colorStep -= (1.0 - r);
				r = 1.0;
			}
			else {
				r += colorStep;
				break;
			}
		}
		else if (b > 0.0 && r == 1.0) {
			if (b - colorStep < 0.0) {
				colorStep -= b;
				b = 0.0;
			}
			else {
				b -= colorStep;
				break;
			}
		}
	}
	return vec4(r * brightness, g * brightness, b * brightness, 1.0);
}

#elif FLOW_COLOR_TYPE == 3

vec3 HSVToRGB(float h, float s, float v) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(vec3(h, h, h) + K.xyz) * 6.0 - K.www);
    return v * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), s);
}

vec4 rgbStep(float index) {
	if (index == 0)
		return vec4(0.0, 0.0, 0.0, 1.0);
	
	float r = 0.0;
	float g = 1.0;
	float b = 0.5333;

	float colorStep = index / colorAccuracy;

	while (true) {
		if (r == 1.0 && g < 1.0 && b == 0.0) {
			if (g + colorStep > 1.0) {
				colorStep -= (1.0 - g);
				g = 1.0;
			}
			else {
				g += colorStep;
				break;
			}
		}
		else if (r > 0.0 && g == 1.0) {
			if (r - colorStep < 0.0) {
				colorStep -= r;
				r = 0.0;
			}
			else {
				r -= colorStep;
				break;
			}
		}
		else if (g == 1.0 && b < 1.0) {
			if (b + colorStep > 1.0) {
				colorStep -= (1.0 - b);
				b = 1.0;
			}
			else {
				b += colorStep;
				break;
			}
		}
		else if (g > 0.0 && b == 1.0) {
			if (g - colorStep < 0.0) {
				colorStep -= g;
				g = 0.0;
			}
			else {
				g -= colorStep;
				break;
			}
		}
		else if (b == 1.0 && r < 1.0) {
			if (r + colorStep > 1.0) {
				colorStep -= (1.0 - r);
				r = 1.0;
			}
			else {
				r += colorStep;
				break;
			}
		}
		else if (b > 0.0 && r == 1.0) {
			if (b - colorStep < 0.0) {
				colorStep -= b;
				b = 0.0;
			}
			else {
				b -= colorStep;
				break;
			}
		}
	}
	return vec4(r, g, b, 1.0);
}

vec4 flowColor(uint index, double escapeReal, double escapeImag) {
	if (index == 0)
		return vec4(0.0f, 0.0f, 0.0f, 1.0f);

	float smoothStep = float(index) + 1.0f - 0.42 * log(log(sqrt(float(escapeReal * escapeReal + escapeImag * escapeImag)))) / 0.301029996f;

	return rgbStep(smoothStep);

	// if (hue < 0.0f)
	// 	return vec4(1.0, 0, 0, 1.0);
	// else if (hue > 1.0f)
	// 	return vec4(0.0, 0.0, 1.0, 1.0);
	// while (hue > 360.0)
	// 	hue -= 360.0;
    // while (hue < 0.0)
	// 	hue += 360.0;

	// float r = hue;
	// float g = r * r;
	// float b = r * g;
	// return vec4(r, g, b, 1.0);

	// vec3 color = HSVToRGB(hue, 0.8f, 1.0f);
	// return vec4(color.x, color.y, color.z, 1.0);
}

#endif

void main() {
	double real = zoomScale * (double(gl_FragCoord.x) + 0.5) / windowSize.x + numberStart.x;
	double imag = (zoomScale * (double(gl_FragCoord.y) + 0.5) + numberStart.y * windowSize.y) / windowSize.x;
	
#if FLOW_COLOR_TYPE == 3

	double escapeReal;
	double escapeImag;
	uint calc = calcFractal(real, imag, escapeReal, escapeImag);
	fragColor = flowColor(calc, escapeReal, escapeImag);

#else

	uint calc = calcFractal(real, imag);
	fragColor = flowColor(calc);

#endif
}
