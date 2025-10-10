#pragma once
#ifndef MANDELBROT_MODEL_RK45_INCLUDED
#define MANDELBROT_MODEL_RK45_INCLUDED

#include "model.h"

class RK45Model : public virtual Model {
protected:
    // RK45 Parameters
    int maxSteps = 10'000;
    int maxSameSteps = 30u;
    float minStepSize = 1e-12f;
    float atolExponent = -5.0f; // corresponds to 1e-5
    float rtolExponent = -5.0f; // corresponds to 1e-5

    // UI Variables
    int maxStepsMin = 1;
    int maxStepsMax = 100'000;

public:
    RK45Model(const std::string& _name, Shader&& _shader) : Model(_name, std::move(_shader)) { }
    RK45Model(const RK45Model& other) :
        Model(other),
        maxSteps(other.maxSteps), maxSameSteps(other.maxSameSteps),
        minStepSize(other.minStepSize), atolExponent(other.atolExponent), rtolExponent(other.rtolExponent),
        maxStepsMin(other.maxStepsMin), maxStepsMax(other.maxStepsMax)
        { }

    RK45Model& operator=(const RK45Model& other) = delete;
    RK45Model& operator=(RK45Model&& other) = delete;

    virtual void applyUniformVariables() override;
    virtual void imGuiFrame() override;
    virtual void imGuiScreenshotFrame() override;
    virtual std::unique_ptr<Model> clone() const override;
    virtual void makeScreenshotModel() override;
    virtual void makeScreenshotModel(const Model& otherScreenshotModel);

private:
    void imGuiFrameHelper();
    void setDefaultScreenshotParameters();

};

#endif
