#include "model_super_sampling.h"

#include <algorithm> // for std::find_if
#include <utility> // for std::pair
#include <array> // for std::array

#include <ImGui/imgui.h>

void SuperSamplingModel::initDefines() {
    this->Model::initDefines();

    this->shader.define("SUPER_SAMPLING", std::to_string(ssMode));
}

void SuperSamplingModel::applyUniformVariables() {
    this->Model::applyUniformVariables();

	this->shader.setFloat("MEAN_DIFF_TOL", ssMeanDiffTolerance);
	this->shader.setFloat("ABS_SE_TOL", ssAbsoluteStandardErrorTolerance);
	this->shader.setFloat("REL_SE_TOL", ssRelativeStandardErrorTolerance);
}

void SuperSamplingModel::imGuiFrameHelper() {
    static const std::array<std::pair<const char*, SuperSamplingModel::Mode>, 10> options = {{
        std::make_pair("Adaptive", SuperSamplingModel::ADAPTIVE),
        std::make_pair("Off", SuperSamplingModel::OFF),
        std::make_pair("2", SuperSamplingModel::_2),
        std::make_pair("4", SuperSamplingModel::_4),
        std::make_pair("6", SuperSamplingModel::_6),
        std::make_pair("8", SuperSamplingModel::_8),
        std::make_pair("12", SuperSamplingModel::_12),
        std::make_pair("16", SuperSamplingModel::_16),
        std::make_pair("16 (pmj)", SuperSamplingModel::_16_PMJ),
        std::make_pair("32 (pmj)", SuperSamplingModel::_32_PMJ)
    }};

    
    if (ImGui::CollapsingHeader("Super Sampling (Anti-Aliasing)", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto it = std::find_if(options.begin(), options.end(),
            [currentSSMode = this->ssMode](const auto& p) {
                return p.second == currentSSMode;
            });
        long index = (it != options.end()) ? std::distance(options.begin(), it) : -1L;

        if (ImGui::BeginCombo("Mode", options[static_cast<unsigned long>(index)].first)) {
            for (const auto& option : options) {
                bool isSelected = (this->ssMode == option.second);
                if (ImGui::Selectable(option.first, isSelected)) {
                    this->ssMode = option.second;
                    this->shader.define("SUPER_SAMPLING", std::to_string(this->ssMode));
                    this->shader.recompile();
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (this->ssMode == SuperSamplingModel::ADAPTIVE) {
            ImGui::Text("Adaptive Super Sampling: ");
            if (ImGui::SliderFloat("Mean Tol", &ssMeanDiffTolerance, 0.0f, 0.4f)) {
                this->shader.setFloat("MEAN_DIFF_TOL", ssMeanDiffTolerance);
            }
            if (ImGui::SliderFloat("Abs SE Tol", &ssAbsoluteStandardErrorTolerance, 0.0f, 0.4f)) {
                this->shader.setFloat("ABS_SE_TOL", ssAbsoluteStandardErrorTolerance);
            }
            if (ImGui::SliderFloat("Rel SE Tol", &ssRelativeStandardErrorTolerance, 0.0f, 0.4f)) {
                this->shader.setFloat("REL_SE_TOL", ssRelativeStandardErrorTolerance);
            }
        }
    }
}

void SuperSamplingModel::imGuiFrame() {
    this->Model::imGuiFrame();

    this->imGuiFrameHelper();
}

void SuperSamplingModel::imGuiScreenshotFrame() {
    this->Model::imGuiScreenshotFrame();

    this->imGuiFrameHelper();
}

std::unique_ptr<Model> SuperSamplingModel::clone() const {
    return std::make_unique<SuperSamplingModel>(*this);
}

void SuperSamplingModel::setDefaultScreenshotParameters() {
    this->ssMode = SuperSamplingModel::_32_PMJ;
    this->ssMeanDiffTolerance = 0.003f;
    this->ssAbsoluteStandardErrorTolerance = 0.004f;
    this->ssRelativeStandardErrorTolerance = 0.01f;
}

void SuperSamplingModel::makeScreenshotModel() {
    this->Model::makeScreenshotModel();

    this->setDefaultScreenshotParameters();
}

void SuperSamplingModel::makeScreenshotModel(const Model& otherScreenshotModel) {
    this->Model::makeScreenshotModel();

    const SuperSamplingModel* otherScreenshotSSModel = dynamic_cast<const SuperSamplingModel*>(&otherScreenshotModel);
    if (otherScreenshotSSModel == nullptr) {
        this->setDefaultScreenshotParameters();
        return;
    }

    this->ssMode = otherScreenshotSSModel->ssMode;
    this->ssMeanDiffTolerance = otherScreenshotSSModel->ssMeanDiffTolerance;
    this->ssAbsoluteStandardErrorTolerance = otherScreenshotSSModel->ssAbsoluteStandardErrorTolerance;
    this->ssRelativeStandardErrorTolerance = otherScreenshotSSModel->ssRelativeStandardErrorTolerance;
}
