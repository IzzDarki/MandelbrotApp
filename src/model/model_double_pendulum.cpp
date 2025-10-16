#include "model_double_pendulum.h"

#include <ImGui/imgui.h>

#include "../app_utility.h"

DoublePendulumModel::DoublePendulumModel() :
    Model(
        "Double Pendulum",
        Shader("../res/vertex_shader.glsl", "../res/fragment_shader_double_pendulum.glsl") // Don't compile and link shader (not responsibility of the model)
    ),
    SuperSamplingModel(
        "Double Pendulum",
        Shader("../res/vertex_shader.glsl", "../res/fragment_shader_double_pendulum.glsl") // Don't compile and link shader (not responsibility of the model)
    ),
    RK45Model(
        "Double Pendulum",
        Shader("../res/vertex_shader.glsl", "../res/fragment_shader_double_pendulum.glsl") // Don't compile and link shader (not responsibility of the model)
    )
    { }

DoublePendulumModel::DoublePendulumModel(const DoublePendulumModel& other)
    : Model(other),
      SuperSamplingModel(other),
      RK45Model(other),
      simulationEndTime(other.simulationEndTime),
      v1Start(other.v1Start),
      v2Start(other.v2Start),
      weightConstant(other.weightConstant),
      length1(other.length1),
      length2(other.length2),
      mass1(other.mass1),
      mass2(other.mass2)
    { }

void DoublePendulumModel::applyUniformVariables() {
    this->SuperSamplingModel::applyUniformVariables();
    this->RK45Model::applyUniformVariables();

	// Double Pendulum
	this->shader.setFloat("t_end", this->simulationEndTime);
	this->shader.setFloat("v1_start", this->v1Start);
	this->shader.setFloat("v2_start", this->v2Start);
	this->shader.setFloat("g", this->weightConstant);
	this->shader.setFloat("l1", this->length1);
	this->shader.setFloat("l2", this->length2);
	this->shader.setFloat("m1", this->mass1);
	this->shader.setFloat("m2", this->mass2);
}

void DoublePendulumModel::imGuiFrame() {
    this->SuperSamplingModel::imGuiFrame();
    this->RK45Model::imGuiFrame();

    if (ImGui::CollapsingHeader("Double Pendulum", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGuiFlexibleSliderFloat("End-Time", &this->simulationEndTime, &this->simulationEndTimeMin, &this->simulationEndTimeMax, 1.0f, 1.3f)) {
            this->shader.setFloat("t_end", this->simulationEndTime);
        }
        if (ImGui::SliderFloat("v1-Start", &this->v1Start, -1.0f, +1.0f)) {
            this->shader.setFloat("v1_start", this->v1Start);
        }
        if (ImGui::SliderFloat("v2-Start", &this->v2Start, -1.0f, +1.0f)) {
            this->shader.setFloat("v2_start", this->v2Start);
        }
        if (ImGui::SliderFloat("g", &this->weightConstant, -5.0, 40.0)) {
            this->shader.setFloat("g", this->weightConstant);
        }
        if (ImGui::SliderFloat("l1", &this->length1, -1.0, 8.0)) {
            this->shader.setFloat("l1", this->length1);
        }
        if (ImGui::SliderFloat("l2", &this->length2, -1.0, 8.0)) {
            this->shader.setFloat("l2", this->length2);
        }
        if (ImGui::SliderFloat("m1", &this->mass1, -2.0, 8.0)) {
            this->shader.setFloat("m1", this->mass1);
        }
        if (ImGui::SliderFloat("m2", &this->mass2, -2.0, 8.0)) {
            this->shader.setFloat("m2", this->mass2);
        }
    }
}


void DoublePendulumModel::imGuiScreenshotFrame() {
    this->SuperSamplingModel::imGuiScreenshotFrame();
    this->RK45Model::imGuiScreenshotFrame();
}

std::unique_ptr<Model> DoublePendulumModel::clone() const {
    return std::make_unique<DoublePendulumModel>(*this);
}

void DoublePendulumModel::makeScreenshotModel() {
    this->SuperSamplingModel::makeScreenshotModel();
    this->RK45Model::makeScreenshotModel();
}

void DoublePendulumModel::makeScreenshotModel(const Model& otherScreenshotModel) {
    this->SuperSamplingModel::makeScreenshotModel(otherScreenshotModel);
    this->RK45Model::makeScreenshotModel(otherScreenshotModel);

    // No need to copy assign any attributes from the otherScreenshotModel,
    // since all attributes are only relevant to the live model
}

void DoublePendulumModel::updateWithLiveModel(const Model& liveModel) {
    this->SuperSamplingModel::updateWithLiveModel(liveModel);
    this->RK45Model::updateWithLiveModel(liveModel);

    const DoublePendulumModel* liveDoublePendulumModel = dynamic_cast<const DoublePendulumModel*>(&liveModel);
    if (liveDoublePendulumModel == nullptr) { // liveModel is not a DoublePendulumModel
        return;
    }

    this->simulationEndTime = liveDoublePendulumModel->simulationEndTime;
    this->v1Start = liveDoublePendulumModel->v1Start;
    this->v2Start = liveDoublePendulumModel->v2Start;
    this->weightConstant = liveDoublePendulumModel->weightConstant;
    this->length1 = liveDoublePendulumModel->length1;
    this->length2 = liveDoublePendulumModel->length2;
    this->mass1 = liveDoublePendulumModel->mass1;
    this->mass2 = liveDoublePendulumModel->mass2;
}
