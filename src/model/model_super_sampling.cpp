#include "model_super_sampling.h"

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
    if (ImGui::CollapsingHeader("Super Sampling (Anti-Aliasing)")) {
        if (ImGui::Selectable("Adaptive", (ssMode == SuperSamplingModel::ADAPTIVE))) {
            ssMode = SuperSamplingModel::ADAPTIVE;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::ADAPTIVE));
            this->shader.recompile();
        }
        if (ImGui::Selectable("Off", (ssMode == SuperSamplingModel::OFF))) {
            ssMode = SuperSamplingModel::OFF;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::OFF));
            this->shader.recompile();
        }
        if (ImGui::Selectable("2", (ssMode == SuperSamplingModel::_2))) {
            ssMode = SuperSamplingModel::_2;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_2));
            this->shader.recompile();
        }
        if (ImGui::Selectable("4", (ssMode == SuperSamplingModel::_4))) {
            ssMode = SuperSamplingModel::_4;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_4));
            this->shader.recompile();
        }
        if (ImGui::Selectable("6", (ssMode == SuperSamplingModel::_6))) {
            ssMode = SuperSamplingModel::_6;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_6));
            this->shader.recompile();
        }
        if (ImGui::Selectable("8", (ssMode == SuperSamplingModel::_8))) {
            ssMode = SuperSamplingModel::_8;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_8));
            this->shader.recompile();
        }
        if (ImGui::Selectable("12", (ssMode == SuperSamplingModel::_12))) {
            ssMode = SuperSamplingModel::_12;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_12));
            this->shader.recompile();
        }
        if (ImGui::Selectable("16", (ssMode == SuperSamplingModel::_16))) {
            ssMode = SuperSamplingModel::_16;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_16));
            this->shader.recompile();
        }
        if (ImGui::Selectable("16 (pmj)", (ssMode == SuperSamplingModel::_16_PMJ))) {
            ssMode = SuperSamplingModel::_16_PMJ;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_16_PMJ));
            this->shader.recompile();
        }
        if (ImGui::Selectable("32 (pmj)", (ssMode == SuperSamplingModel::_32_PMJ))) {
            ssMode = SuperSamplingModel::_32_PMJ;
            this->shader.define("SUPER_SAMPLING", std::to_string(SuperSamplingModel::_32_PMJ));
            this->shader.recompile();
        }

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
