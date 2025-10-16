#include "model_mandelbrot.h"

#include <string> // for stoi

#include <ImGui/imgui.h>

MandelbrotModel::MandelbrotModel()
    : Model("Mandelbrot", Shader("../res/vertex_shader.glsl", "../res/fragment_shader_mandelbrot.glsl"))
{
    this->setColorMap(MandelbrotModel::RainbowSmooth);
}

MandelbrotModel::MandelbrotModel(const MandelbrotModel& other)
    : Model(other),
      colorAccuracy(other.colorAccuracy)
{
    // strncpy(this->codeDivergenceCriterion, other.codeDivergenceCriterion, 1000); // copy at most 1000 characters
    // this->codeDivergenceCriterion[1000 - 1] = '\0'; // ensure null termination

    // strncpy(this->codeCalculateNextSequenceTerm, other.codeCalculateNextSequenceTerm, 2000); // copy at most 2000 characters
    // this->codeCalculateNextSequenceTerm[2000 - 1] = '\0'; // ensure null termination
}

void MandelbrotModel::applyUniformVariables() {
    this->Model::applyUniformVariables();

    this->shader.setUInt("maxIterations", static_cast<uint>(this->maxIterations));
    this->shader.setUInt("colorAccuracy", static_cast<uint>(this->colorAccuracy));
}

void MandelbrotModel::imGuiFrame() {
    this->Model::imGuiFrame();

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

    if (ImGui::SliderInt("Color accuracy", &this->colorAccuracy, 1, 1'000)) {
    	this->shader.setUInt("colorAccuracy", static_cast<uint>(this->colorAccuracy));
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
    this->Model::imGuiScreenshotFrame();

}

std::unique_ptr<Model> MandelbrotModel::clone() const {
    return std::make_unique<MandelbrotModel>(*this);
}


void MandelbrotModel::makeScreenshotModel() {
    this->Model::makeScreenshotModel();

    this->setDefaultScreenshotParameters();
}

void MandelbrotModel::makeScreenshotModel(const Model& otherScreenshotModel) {
    this->Model::makeScreenshotModel(otherScreenshotModel);

    const MandelbrotModel* otherScreenshotMandelbrotModel = dynamic_cast<const MandelbrotModel*>(&otherScreenshotModel);
    if (otherScreenshotMandelbrotModel == nullptr) {
        this->setDefaultScreenshotParameters();
        return;
    }

    this->maxIterations = otherScreenshotMandelbrotModel->maxIterations;
}


void MandelbrotModel::updateWithLiveModel(const Model& liveModel) {
    this->Model::updateWithLiveModel(liveModel);

    const MandelbrotModel* liveMandelbrotModel = dynamic_cast<const MandelbrotModel*>(&liveModel);
    if (liveMandelbrotModel == nullptr) { // liveModel is not a MandelbrotModel
        return;
    }

    this->colorAccuracy = liveMandelbrotModel->colorAccuracy;
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