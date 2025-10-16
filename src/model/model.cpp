#include "model.h"

void Model::applyUniformVariables() { }
void Model::imGuiFrame() { }
void Model::imGuiScreenshotFrame() { }
std::unique_ptr<Model> Model::clone() const {
    return std::make_unique<Model>(*this);
}
void Model::makeScreenshotModel() { }
void Model::makeScreenshotModel(const Model& otherScreenshotModel) { (void)otherScreenshotModel; }
void Model::updateWithLiveModel(const Model& liveModel) { (void)liveModel; }
Model::~Model() { }
