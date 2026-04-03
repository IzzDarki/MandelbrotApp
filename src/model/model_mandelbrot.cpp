#include "model_mandelbrot.h"

#include <string> // for stoi

#include <ImGui/imgui.h>

MandelbrotModel::MandelbrotModel()
    : Model("MandelbrotModel", Shader("../res/vertex_shader.glsl", "../res/fragment_shader_mandelbrot.glsl")),
      ColormapModel("MandelbrotModel", Shader("../res/vertex_shader.glsl", "../res/fragment_shader_mandelbrot.glsl"))
{
    this->selectColormap("Cyclic", "cet_colorwheel"); // Cyclic colormap as default

    if (this->useDoublePrecision) {
        this->shader.define("USE_DOUBLE", "");
    }

    this->setColorMap(MandelbrotModel::RainbowSmooth);
}

MandelbrotModel::MandelbrotModel(const MandelbrotModel& other)
    : Model(other),
      ColormapModel(other),
      maxIterations(other.maxIterations),
      colorScale(other.colorScale),
      useDoublePrecision(other.useDoublePrecision),
      sliceValue(other.sliceValue),
      sliceFactor(other.sliceFactor)
{
    // strncpy(this->codeDivergenceCriterion, other.codeDivergenceCriterion, 1000); // copy at most 1000 characters
    // this->codeDivergenceCriterion[1000 - 1] = '\0'; // ensure null termination

    // strncpy(this->codeCalculateNextSequenceTerm, other.codeCalculateNextSequenceTerm, 2000); // copy at most 2000 characters
    // this->codeCalculateNextSequenceTerm[2000 - 1] = '\0'; // ensure null termination
}

void MandelbrotModel::applyUniformVariables() {
    this->ColormapModel::applyUniformVariables();

    this->shader.setUInt("maxIterations", static_cast<uint>(this->maxIterations));
    this->shader.setFloat("colorScale", this->colorScale);
}

void MandelbrotModel::imGuiFrame() {
    this->ColormapModel::imGuiFrame();

    ImGui::Text("Color: ");
    ImGui::SameLine();
    if (ImGui::SmallButton("Rainbow")) {
        this->setColorMap(MandelbrotModel::Rainbow);
    	this->shader.recompile();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("RainbowSmooth")) {
        this->setColorMap(MandelbrotModel::RainbowSmooth);
    	this->shader.recompile();
    }
    
    // new line
    if (ImGui::SmallButton("Black/White")) {
        this->setColorMap(MandelbrotModel::BlackWhite);
    	this->shader.recompile();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Glowing")) {
        this->setColorMap(MandelbrotModel::Glowing);
    	this->shader.recompile();
    }

    if (ImGui::SliderFloat("Color Scale", &this->colorScale, 0.5f, 300.0f)) {
    	this->shader.setFloat("colorScale", this->colorScale);
    }

    if (ImGui::SliderInt("Slice Value for Black/White", &this->sliceValue, 0, this->maxIterations)) {
        this->shader.setUInt("sliceValue", static_cast<uint>(this->sliceValue));
    }

    if (ImGui::SliderFloat("Slice Factor for Black/White", &this->sliceFactor, 0.0f, 1.0f)) {
        this->shader.setFloat("sliceFactor", this->sliceFactor);
    }

    if (ImGui::Checkbox("Use Double Precision", &this->useDoublePrecision)) {
        if (this->useDoublePrecision) {
            this->shader.define("USE_DOUBLE", ""); 
        } else {
            this->shader.undefine("USE_DOUBLE");
        }
        this->shader.recompile();

    }

    this->imGuiScreenshotFrameHelper();

    // Sequence
    // ImGui::Text("Sequence code (1. Divergence criterion, 2. Code to calculate next term in sequence)");
    // ImGui::InputTextMultiline("1", codeDivergenceCriterion, IM_ARRAYSIZE(codeDivergenceCriterion),
    // 	ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2.5f)
    // );
    // ImGui::InputTextMultiline("2", codeCalculateNextSequenceTerm, IM_ARRAYSIZE(codeCalculateNextSequenceTerm),
    // 	ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3.5f)
    // );
    // if (ImGui::SmallButton("Apply")) {
    // 	this->shader.define(CODE_DIVERGENCE_CRITERION, std::string(codeDivergenceCriterion));
    // 	this->shader.define(CODE_CALCULATE_NEXT_SEQUENCE_TERM, std::string(codeCalculateNextSequenceTerm));
    // 	this->shader.recompile();
    // }
    // ImGui::NewLine();
}

void MandelbrotModel::imGuiScreenshotFrame() {
    this->ColormapModel::imGuiScreenshotFrame();
    this->imGuiScreenshotFrameHelper();
}

std::unique_ptr<Model> MandelbrotModel::clone() const {
    return std::make_unique<MandelbrotModel>(*this);
}


void MandelbrotModel::makeScreenshotModel() {
    this->ColormapModel::makeScreenshotModel();

    this->setDefaultScreenshotParameters();
}

void MandelbrotModel::makeScreenshotModel(const Model& otherScreenshotModel) {
    this->ColormapModel::makeScreenshotModel(otherScreenshotModel);

    const MandelbrotModel* otherScreenshotMandelbrotModel = dynamic_cast<const MandelbrotModel*>(&otherScreenshotModel);
    if (otherScreenshotMandelbrotModel == nullptr) {
        this->setDefaultScreenshotParameters();
        return;
    }

    this->maxIterations = otherScreenshotMandelbrotModel->maxIterations;
}


void MandelbrotModel::updateWithLiveModel(const Model& liveModel) {
    this->ColormapModel::updateWithLiveModel(liveModel);

    const MandelbrotModel* liveMandelbrotModel = dynamic_cast<const MandelbrotModel*>(&liveModel);
    if (liveMandelbrotModel == nullptr) { // liveModel is not a MandelbrotModel
        return;
    }

    this->colorScale = liveMandelbrotModel->colorScale;
    this->setColorMap(liveMandelbrotModel->getColorMap());
}

MandelbrotModel::ColorMap MandelbrotModel::getColorMap() const {
    return static_cast<MandelbrotModel::ColorMap>(stoi(this->shader.getDefine(FLOW_COLOR_TYPE))); // stoi = string to int
}

void MandelbrotModel::setColorMap(ColorMap colorMap) {
    this->shader.define(FLOW_COLOR_TYPE, std::to_string(colorMap));
}


void MandelbrotModel::imGuiScreenshotFrameHelper() {
    if (ImGui::SliderInt("Max Iterations", &this->maxIterations, 0, 5'000)) {
        this->shader.setUInt("maxIterations", static_cast<uint>(this->maxIterations));
    }
}

void MandelbrotModel::setDefaultScreenshotParameters() {
    this->maxIterations = 5'000;
}
