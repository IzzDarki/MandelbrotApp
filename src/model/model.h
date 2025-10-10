#pragma once
#ifndef MANDELBROT_MODEL_INCLUDED
#define MANDELBROT_MODEL_INCLUDED

#include <memory> // for std::unique_ptr

#include "../shader.h"

class Model {
public:
    std::string name;
    Shader shader;

    Model(const std::string& _name, Shader&& _shader) : name(_name), shader(std::move(_shader)) { }
    Model(const Model& other) : name(other.name), shader(other.shader) { }
    Model(Model&& other) = delete;

    Model& operator=(const Model& other) = delete;
    Model& operator=(Model&& other) = delete;

    virtual void initDefines();

    virtual void applyUniformVariables();

    virtual void imGuiFrame();

    /** ImGui Frame for the screenshot UI */
    virtual void imGuiScreenshotFrame();

    /** Clone the model into a separate unique_ptr */
    virtual std::unique_ptr<Model> clone() const;

    /** Modify this model to be used as a screenshot model, i. e., overwrite certain attributes to higher quality */
    virtual void makeScreenshotModel();

    /** Modify this model to be used as a screenshot model, i.e ., overwrite certain attributes to higher quality
        Use the parameters of a given screenshot model as far as the class structures of it and this are the same */
    virtual void makeScreenshotModel(const Model& otherScreenshotModel);

    /** Update this model (which is assumed to be used for screenshots) with values from the "live model",
        which must be of the same child class for this method to have the full effect (otherwise only the parts until the farthest common child is updated).
        This is different from a copy assignment in that this model will keep certain attributes fixed
        (usually the same ones that `makeScreenshotModel` touches) */
    virtual void updateWithLiveModel(const Model& liveModel);

    virtual ~Model();
};

#endif
