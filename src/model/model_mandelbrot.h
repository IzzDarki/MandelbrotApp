#pragma once
#ifndef MANDELBROT_MODEL_MANDELBROT_INCLUDED
#define MANDELBROT_MODEL_MANDELBROT_INCLUDED

#include <string>

#include "model.h"

class MandelbrotModel : virtual public Model {
public:
    enum ColorMap {
        Rainbow = 0,
        BlackWhite = 1,
        Glowing = 2,
        RainbowSmooth = 3
    };

public:
    MandelbrotModel();
    MandelbrotModel(const MandelbrotModel& other);

    virtual void applyUniformVariables() override;
    virtual void imGuiFrame() override;
    virtual void imGuiScreenshotFrame() override;
    virtual std::unique_ptr<Model> clone() const override;
    virtual void makeScreenshotModel() override;
    virtual void makeScreenshotModel(const Model& otherScreenshotModel) override;
    virtual void updateWithLiveModel(const Model& liveModel) override;

    ColorMap getColorMap() const;
    void setColorMap(ColorMap colorMap);

public:
    constexpr static const char* FLOW_COLOR_TYPE = "FLOW_COLOR_TYPE";
    // constexpr static const char* CODE_DIVERGENCE_CRITERION = "CODE_DIVERGENCE_CRITERION";
    // constexpr static const char* CODE_CALCULATE_NEXT_SEQUENCE_TERM = "CODE_CALCULATE_NEXT_SEQUENCE_TERM";

    int maxIterations = 400;
    int colorAccuracy = 10; // TODO Just use color maps
    // char codeDivergenceCriterion[1000] = "real*real + imag*imag > 4";
    // char codeCalculateNextSequenceTerm[2000] = "real = real*real - imag*imag + startReal;\nimag = 2 * realTemp * imag + startImag;";

protected:
    void imGuiScreenshotFrameHelper();
    void setDefaultScreenshotParameters();
};

#endif
