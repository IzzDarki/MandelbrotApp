#pragma once
#ifndef MANDELBROT_MODEL_DOUBLE_PENDULUM_INCLUDED
#define MANDELBROT_MODEL_DOUBLE_PENDULUM_INCLUDED

#include "model_rk45.h"
#include "model_super_sampling.h"

class DoublePendulumModel : public virtual SuperSamplingModel, public virtual RK45Model {
protected:
    // Double Pendulum Parameters
    float simulationEndTime = 3.0f;
    float v1Start = 0.0f;
    float v2Start = 0.0f;
    float weightConstant = 9.81f;
    float length1 = 1.0f;
    float length2 = 1.0f;
    float mass1 = 1.0f;
    float mass2 = 1.0f;

    // UI Variables
    float simulationEndTimeMin = 0.0f;
    float simulationEndTimeMax = 10.0f;

public:
    DoublePendulumModel();
    DoublePendulumModel(const DoublePendulumModel& other);

    DoublePendulumModel& operator=(const DoublePendulumModel& other) = delete;
    DoublePendulumModel& operator=(DoublePendulumModel&& other) = delete;

    virtual void applyUniformVariables() override;
    virtual void imGuiFrame() override;
    virtual void imGuiScreenshotFrame() override;
    virtual std::unique_ptr<Model> clone() const override;
    virtual void makeScreenshotModel() override;
    virtual void makeScreenshotModel(const Model& otherScreenshotModel) override;
    virtual void updateWithLiveModel(const Model& liveModel) override;
};

#endif
