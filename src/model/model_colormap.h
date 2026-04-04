#pragma once
#ifndef MANDELBROT_MODEL_COLORMAP_INCLUDED
#define MANDELBROT_MODEL_COLORMAP_INCLUDED

#include "model.h"

#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../colormaps.h"

class ColormapModel : public virtual Model {
public:
    ColormapModel(const std::string& _name, Shader&& _shader);
    ColormapModel(const ColormapModel& other);

    // virtual void applyUniformVariables() override;
    virtual void imGuiFrame() override;
    virtual void imGuiScreenshotFrame() override;
    virtual std::unique_ptr<Model> clone() const override;
    virtual void makeScreenshotModel() override;
    virtual void makeScreenshotModel(const Model& otherScreenshotModel) override;
    virtual void drawCall() override;
    
    void selectColormap(const std::string& group, const std::string& name);

protected:
    void setDefaultScreenshotParameters();
    void initializeColormapTexture(const std::string& defaultGroup, const std::string& defaultName);
    static void applyWrapMode(const std::string& group);

    std::string selectedColormapGroup;
    std::string selectedColormapName;
    std::shared_ptr<GLuint> colormapTexture;
};


#endif
