#pragma once
#ifndef MANDELBROT_MODEL_COLORMAP_OLD_INCLUDED
#define MANDELBROT_MODEL_COLORMAP_OLD_INCLUDED

#include "model.h"

class ColormapModelOld : public virtual Model {
public:
    enum Colormap {
        NONE = 0,
        VIRIDIS = 1,
        INFERNO = 2,
        PLASMA = 3,
        MAGMA = 4,
        TURBO = 5,
    };

public:
    ColormapModel(const std::string& _name, Shader&& _shader);
    ColormapModel(const ColormapModel& other);

    // virtual void applyUniformVariables() override;
    virtual void imGuiFrame() override;
    virtual void imGuiScreenshotFrame() override;
    virtual std::unique_ptr<Model> clone() const override;
    virtual void makeScreenshotModel() override;
    virtual void makeScreenshotModel(const Model& otherScreenshotModel) override;
    
    /** Set super sampling mode in the shader, but don't recompile it */
    void setSelectedColormap(Colormap newColormap);
    Colormap getSelectedColormap() const;

protected:
    void imGuiFrameHelper();
    void setDefaultScreenshotParameters();

};


#endif
