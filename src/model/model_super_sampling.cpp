#include "model_super_sampling.h"

#include <algorithm> // for std::find_if
#include <utility> // for std::pair
#include <array> // for std::array
#include <string> // for stoi

#include <ImGui/imgui.h>


SuperSamplingModel::SuperSamplingModel(const std::string& _name, Shader&& _shader)
    : Model(_name, std::move(_shader))
{
    this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_2));
}

SuperSamplingModel::SuperSamplingModel(const SuperSamplingModel& other)
    : Model(other),
      ssMeanDiffTolerance(other.ssMeanDiffTolerance),
      ssAbsoluteStandardErrorTolerance(other.ssAbsoluteStandardErrorTolerance),
      ssRelativeStandardErrorTolerance(other.ssRelativeStandardErrorTolerance)
{ }

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
        const auto currentSSMode = this->getSSMode();
        auto it = std::find_if(options.begin(), options.end(),
            [currentSSMode](const auto& p) {
                return p.second == currentSSMode;
            });
        long index = (it != options.end()) ? std::distance(options.begin(), it) : -1L;

        if (ImGui::BeginCombo("Mode", options[static_cast<unsigned long>(index)].first)) {
            for (const auto& option : options) {
                bool isSelected = (currentSSMode == option.second);
                if (ImGui::Selectable(option.first, isSelected)) {
                    this->setSSMode(option.second);
                    this->shader.recompile(); // needed, because super sampling mode is a #define
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (currentSSMode == SuperSamplingModel::ADAPTIVE) {
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
    this->ssMeanDiffTolerance = 0.003f;
    this->ssAbsoluteStandardErrorTolerance = 0.004f;
    this->ssRelativeStandardErrorTolerance = 0.01f;
    this->setSSMode(SuperSamplingModel::_32_PMJ);
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

    this->ssMeanDiffTolerance = otherScreenshotSSModel->ssMeanDiffTolerance;
    this->ssAbsoluteStandardErrorTolerance = otherScreenshotSSModel->ssAbsoluteStandardErrorTolerance;
    this->ssRelativeStandardErrorTolerance = otherScreenshotSSModel->ssRelativeStandardErrorTolerance;
    this->setSSMode(otherScreenshotSSModel->getSSMode());
}


SuperSamplingModel::Mode SuperSamplingModel::getSSMode() const {
    return static_cast<SuperSamplingModel::Mode>(stoi(this->shader.getDefine("SUPER_SAMPLING"))); // stoi = string to int
}

void SuperSamplingModel::setSSMode(SuperSamplingModel::Mode mode) {
    this->shader.define("SUPER_SAMPLING", std::to_string(mode));
}
