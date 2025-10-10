#pragma once
#ifndef MANDELBROT_MODEL_SUPER_SAMPLING_INCLUDED
#define MANDELBROT_MODEL_SUPER_SAMPLING_INCLUDED

#include "model.h"

class SuperSamplingModel : public virtual Model {
public:
    enum Mode {
        ADAPTIVE = 0,
        OFF = 1,
        _2 = 2,
        _4 = 4,
        _6 = 6,
        _8 = 8,
        _12 = 12,
        _16 = 16,
        _16_PMJ = 1601,
        _32_PMJ = 32,
    };

public:
    SuperSamplingModel(const std::string& _name, Shader&& _shader) : Model(_name, std::move(_shader)) { }
    SuperSamplingModel(const SuperSamplingModel& other) :
        Model(other),
        ssMode(other.ssMode),
        ssMeanDiffTolerance(other.ssMeanDiffTolerance),
        ssAbsoluteStandardErrorTolerance(other.ssAbsoluteStandardErrorTolerance),
        ssRelativeStandardErrorTolerance(other.ssRelativeStandardErrorTolerance)
    { }

    virtual void initDefines() override;
    virtual void applyUniformVariables() override;
    virtual void imGuiFrame() override;
    virtual void imGuiScreenshotFrame() override;
    virtual std::unique_ptr<Model> clone() const override;
    virtual void makeScreenshotModel() override;
    virtual void makeScreenshotModel(const Model& otherScreenshotModel) override;


public:
    Mode ssMode = _2;
    float ssMeanDiffTolerance = 0.003f;
    float ssAbsoluteStandardErrorTolerance = 0.004f;
    float ssRelativeStandardErrorTolerance = 0.01f;

private:
    void imGuiFrameHelper();
    void setDefaultScreenshotParameters();

};


#endif
