#include "model_rk45.h"

#include <format>
#include <cmath>

#include <ImGui/imgui.h>

#include "../app_utility.h"

void RK45Model::applyUniformVariables() {
    this->Model::applyUniformVariables();

	this->shader.setUInt("MAX_STEPS", static_cast<uint>(this->maxSteps));
	this->shader.setUInt("MAX_SAME_STEPS", static_cast<uint>(this->maxSameSteps));
	this->shader.setFloat("MIN_TAU", this->minStepSize);
	this->shader.setFloat("atol", std::pow(10.0f, this->atolExponent));
	this->shader.setFloat("rtol", std::pow(10.0f, this->rtolExponent));
}

void RK45Model::imGuiFrameHelper() {
    if (ImGui::CollapsingHeader("RK45", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (ImGuiFlexibleSliderInt("Max Iterations", &this->maxSteps, &this->maxStepsMin, &this->maxStepsMax, 1.0f, 1.5f)) {
            this->shader.setUInt("MAX_STEPS", static_cast<uint>(this->maxSteps));
        }

        ImGui::Text("Absolute Tolerance Exponent (10^_)");
        if (ImGui::SliderFloat("##Absolute Tolerance Exponent (10^_)", &this->atolExponent, -14.0, 2.0)) {
            this->shader.setFloat("atol", std::pow(10.0f, this->atolExponent));
        }
        ImGui::SameLine();
        ImGui::Text(std::format("{:.1e}", std::pow(10.0f, this->atolExponent)).c_str());

        ImGui::Text("Relative Tolerance Exponent (10^_)");
        if (ImGui::SliderFloat("##Relative Tolerance Exponent (10^_)", &this->rtolExponent, -14.0, 2.0)) {
            this->shader.setFloat("rtol", std::pow(10.0f, this->rtolExponent));
        }
        ImGui::SameLine();
        ImGui::Text(std::format("{:.1e}", std::pow(10.0f, this->rtolExponent)).c_str());
    }
}

void RK45Model::imGuiFrame() {
    this->Model::imGuiFrame();

    this->imGuiFrameHelper();
}

void RK45Model::imGuiScreenshotFrame() {
    this->Model::imGuiScreenshotFrame();

    this->imGuiFrameHelper();
}

std::unique_ptr<Model> RK45Model::clone() const {
    return std::make_unique<RK45Model>(*this);
}

void RK45Model::setDefaultScreenshotParameters() {
    this->maxSteps = 500'000;
    this->maxSameSteps = 100;
    this->atolExponent = -11.0f;
    this->rtolExponent = -11.0f;
    this->minStepSize = 1e-25f;
}

void RK45Model::makeScreenshotModel() {
    this->Model::makeScreenshotModel();

    this->setDefaultScreenshotParameters();
}

void RK45Model::makeScreenshotModel(const Model& otherScreenshotModel) {
    this->Model::makeScreenshotModel(otherScreenshotModel);

    const RK45Model* otherScreenshotRK45Model = dynamic_cast<const RK45Model*>(&otherScreenshotModel);
    if (otherScreenshotRK45Model == nullptr) {
        this->setDefaultScreenshotParameters();
        return;
    }

    this->maxSteps = otherScreenshotRK45Model->maxSteps;
    this->maxSameSteps = otherScreenshotRK45Model->maxSameSteps;
    this->atolExponent = otherScreenshotRK45Model->atolExponent;
    this->rtolExponent = otherScreenshotRK45Model->rtolExponent;
    this->minStepSize = otherScreenshotRK45Model->minStepSize;
}