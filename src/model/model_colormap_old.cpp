#include "model_colormap_old.h"

#include <algorithm> // for std::find_if
#include <utility> // for std::pair
#include <array> // for std::array
#include <string> // for stoi

#include <ImGui/imgui.h>


ColormapModelOld::ColormapModelOld(const std::string& _name, Shader&& _shader)
    : Model(_name, std::move(_shader))
{
    this->shader.define("SELECTED_COLORMAP", std::to_string(ColormapModelOld::VIRIDIS));
}

ColormapModelOld::ColormapModelOld(const ColormapModelOld& other)
    : Model(other)
{ }

// void ColormapModelOld::applyUniformVariables() {
//     this->Model::applyUniformVariables();
// }

void ColormapModelOld::imGuiFrameHelper() {
    static const std::array<std::pair<const char*, ColormapModelOld::Colormap>, 6> colormapOptions = {{
        std::make_pair("None", ColormapModelOld::NONE),
        std::make_pair("Viridis", ColormapModelOld::VIRIDIS),
        std::make_pair("Inferno", ColormapModelOld::INFERNO),
        std::make_pair("Plasma", ColormapModelOld::PLASMA),
        std::make_pair("Magma", ColormapModelOld::MAGMA),
        std::make_pair("Turbo", ColormapModelOld::TURBO),
    }};

    
    if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto currentColormap = this->getSelectedColormap();
        const auto *it = std::find_if(colormapOptions.begin(), colormapOptions.end(),
            [currentColormap](const auto& p) {
                return p.second == currentColormap;
            });
        const long index = (it != colormapOptions.end()) ? std::distance(colormapOptions.begin(), it) : -1L;

        if (ImGui::BeginCombo("Colormap", colormapOptions[static_cast<unsigned long>(index)].first)) {
            for (const auto& option : colormapOptions) {
                const bool isSelected = (currentColormap == option.second);
                if (ImGui::Selectable(option.first, isSelected)) {
                    this->setSelectedColormap(option.second);
                    this->shader.recompile(); // needed, because colormap is selected using a #define
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
}

void ColormapModelOld::imGuiFrame() {
    this->Model::imGuiFrame();

    this->imGuiFrameHelper();
}

void ColormapModelOld::imGuiScreenshotFrame() {
    this->Model::imGuiScreenshotFrame();

    this->imGuiFrameHelper();
}

std::unique_ptr<Model> ColormapModelOld::clone() const {
    return std::make_unique<ColormapModelOld>(*this);
}

void ColormapModelOld::setDefaultScreenshotParameters() {
    
}

void ColormapModelOld::makeScreenshotModel() {
    this->Model::makeScreenshotModel();

    this->setDefaultScreenshotParameters();
}

void ColormapModelOld::makeScreenshotModel(const Model& otherScreenshotModel) {
    this->Model::makeScreenshotModel();

    const ColormapModelOld* otherScreenshotColormapModelOld = dynamic_cast<const ColormapModelOld*>(&otherScreenshotModel);
    if (otherScreenshotColormapModelOld == nullptr) {
        this->setDefaultScreenshotParameters();
        return;
    }

    this->setSelectedColormap(otherScreenshotColormapModelOld->getSelectedColormap());
}


ColormapModelOld::Colormap ColormapModelOld::getSelectedColormap() const {
    return static_cast<ColormapModelOld::Colormap>(stoi(this->shader.getDefine("SELECTED_COLORMAP"))); // stoi = string to int
}

void ColormapModelOld::setSelectedColormap(ColormapModelOld::Colormap newColormap) {
    this->shader.define("SELECTED_COLORMAP", std::to_string(newColormap));
}
