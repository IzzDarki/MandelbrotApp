#include "real.glsl"

#define evalType rvec4
// needs #define evalType (evalType can be real, rvec2, rvec3, rvec4)
// needs function evaluate(dvec2) -> evalType (evaluation of a pixel-space coordinate)
// needs function evalDiff(evalType a, evalType b) -> real


const dvec2 baseOffset = dvec2(+0.25, +0.25);
#define NUM_OFFSETS_1 1
const dvec2 offsets1[NUM_OFFSETS_1] = { dvec2(-0.25, -0.25) };
#define NUM_OFFSETS_2 2
const dvec2 offsets2[NUM_OFFSETS_2] = { dvec2(+0.25, -0.25), dvec2(-0.25, +0.25) };
#define NUM_OFFSETS_3 2
const dvec2 offsets3[NUM_OFFSETS_3] = { dvec2(+0.25, +0.00), dvec2(-0.25, +0.00) };
#define NUM_OFFSETS_4 7
const dvec2 offsets4[NUM_OFFSETS_4] = { dvec2(+0.00, +0.25), dvec2(+0.00, -0.25), dvec2(0.0, 0.0), dvec2(+0.125, +0.125), dvec2(-0.125, -0.125), dvec2(+0.125, -0.125), dvec2(-0.125, +0.125) };

// A good threshold might be 0.01 * scalar-range
evalType evaluateSuperSampled(dvec2 pixelCenter, real threshold) {

    evalType base = evaluate(pixelCenter + baseOffset);
    // float xDiff = abs(dFdx(baseScalar));
    // float yDiff = abs(dFdy(baseScalar));
    // if (xDiff <= threshold && yDiff <= threshold) {
    //     return baseScalar;
    // }

    // Super sampling
    evalType previousMean = base;
    evalType sum = base;

    for (int i = 0; i < NUM_OFFSETS_1; ++i) {
        sum += evaluate(pixelCenter + offsets1[i]);
    }
    evalType newMean = sum / real(1.0 + NUM_OFFSETS_1);
    if (evalDiff(newMean, previousMean) <= threshold) {
        return newMean;
    }
    previousMean = newMean;

    for (int i = 0; i < NUM_OFFSETS_2; ++i) {
        sum += evaluate(pixelCenter + offsets2[i]);
    }
    newMean = sum / real(1.0 + NUM_OFFSETS_1 + NUM_OFFSETS_2);
    if (evalDiff(newMean, previousMean) <= threshold) {
        return newMean;
    }
    previousMean = newMean;

    for (int i = 0; i < NUM_OFFSETS_3; ++i) {
        sum += evaluate(pixelCenter + offsets3[i]);
    }
    newMean = sum / real(1.0 + NUM_OFFSETS_1 + NUM_OFFSETS_2 + NUM_OFFSETS_3);
    if (evalDiff(newMean, previousMean) <= threshold) {
        return newMean;
    }
    previousMean = newMean;

    for (int i = 0; i < NUM_OFFSETS_4; ++i) {
        sum += evaluate(pixelCenter + offsets4[i]);
    }
    newMean = sum / real(1.0 + NUM_OFFSETS_1 + NUM_OFFSETS_2 + NUM_OFFSETS_3 + NUM_OFFSETS_4);
    return newMean;
}
