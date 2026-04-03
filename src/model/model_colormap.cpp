#include "model_colormap.h"

#include <vector>
#include <algorithm> // for std::find_if
#include <utility> // for std::pair
#include <array> // for std::array
#include <string> // for stoi

#include <ImGui/imgui.h>


ColormapModel::ColormapModel(const std::string& _name, Shader&& _shader)
    : Model(_name, std::move(_shader))
{

    loadColormaps("../res/colormaps.bin");

    // Initialize the shared pointer. The lambda function ensures glDeleteTextures 
    // is called ONLY when the last ColormapModel using this texture is destroyed.
    this->colormapTexture = std::shared_ptr<GLuint>(new GLuint(0), [](GLuint* ptr) {
        if (*ptr != 0) {
            glDeleteTextures(1, ptr);
        }
        delete ptr;
    });

    this->initializeColormapTexture("Perceptually Uniform", "cet_kbc");
}

ColormapModel::ColormapModel(const ColormapModel& other)
    : Model(other)
    , selectedColormapGroup(other.selectedColormapGroup)
    , selectedColormapName(other.selectedColormapName)
    , colormapTexture(other.colormapTexture) // shared pointer copy
{ }

// void ColormapModel::applyUniformVariables() {
//     this->Model::applyUniformVariables();
// }

void ColormapModel::imGuiFrameHelper() {

    if (ImGui::CollapsingHeader("Color", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        // Use the current name as the preview string in the collapsed combo box
        if (ImGui::BeginCombo("Colormap", this->selectedColormapName.c_str())) {
            for (const auto& groupPair : colormaps) {
                const std::string& groupName = groupPair.first;
                
                // Visual separator and unselectable text for the group header
                ImGui::Separator();
                ImGui::TextDisabled("%s", groupName.c_str());

                // 4. Create the selectable items
                for (const auto& mapPair : groupPair.second) {
                    const std::string& mapName = mapPair.first;
                    
                    // Unique ID is required by ImGui just in case two groups have a map with the same name
                    std::string label = mapName + "##" + groupName;
                    
                    const bool isSelected = (this->selectedColormapGroup == groupName && 
                                             this->selectedColormapName == mapName);
                    
                    if (ImGui::Selectable(label.c_str(), isSelected)) {
                        // Immediately apply the new texture to the GPU
                        this->selectColormap(groupName, mapName);
                    }
                    
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
            ImGui::EndCombo();
        }

        // Display a visual preview of the currently selected colormap!
        ImGui::Spacing();
        ImGui::Text("Preview:");
        
        // Pass the raw OpenGL texture ID to ImGui. 
        // We dereference the shared_ptr, then cast it to the pointer type ImGui expects.
        ImGui::Image(
            reinterpret_cast<ImTextureID>(static_cast<intptr_t>(*(this->colormapTexture))),
            ImVec2(ImGui::GetContentRegionAvail().x, 20.0f) // Stretch to full window width, 20px high
        );
        ImGui::Spacing();
    }
}

void ColormapModel::imGuiFrame() {
    this->Model::imGuiFrame();

    this->imGuiFrameHelper();
}

void ColormapModel::imGuiScreenshotFrame() {
    this->Model::imGuiScreenshotFrame();

    this->imGuiFrameHelper();
}

std::unique_ptr<Model> ColormapModel::clone() const {
    return std::make_unique<ColormapModel>(*this);
}

void ColormapModel::setDefaultScreenshotParameters() {
    
}

void ColormapModel::makeScreenshotModel() {
    this->Model::makeScreenshotModel();

    this->setDefaultScreenshotParameters();
}

void ColormapModel::makeScreenshotModel(const Model& otherScreenshotModel) {
    this->Model::makeScreenshotModel();

    const ColormapModel* otherScreenshotColormapModel = dynamic_cast<const ColormapModel*>(&otherScreenshotModel);
    if (otherScreenshotColormapModel == nullptr) {
        this->setDefaultScreenshotParameters();
        return;
    }

    this->selectedColormapGroup = otherScreenshotColormapModel->selectedColormapGroup;
    this->selectedColormapName = otherScreenshotColormapModel->selectedColormapName;
    this->colormapTexture = otherScreenshotColormapModel->colormapTexture;
}

void ColormapModel::drawCall() {
    // bind colormap for render
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *(this->colormapTexture));
    
    // Tell the shader that the "colormap" uniform should look at texture unit 0 // rather use layout(binding = 0) in GLSL
    // GLint loc = glGetUniformLocation(this->shader.shaderProgram, "colormap");
    // glUniform1i(loc, 0); 
}


void ColormapModel::initializeColormapTexture(const std::string& defaultGroup, const std::string& defaultName) {
    this->selectedColormapGroup = defaultGroup;
    this->selectedColormapName = defaultName;

    glGenTextures(1, this->colormapTexture.get());
    glBindTexture(GL_TEXTURE_2D, *(this->colormapTexture));

    // Apply the correct wrap mode for the initial colormap
    ColormapModel::applyWrapMode(defaultGroup);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const float* data = nullptr;
    for (const auto& group : colormaps) {
        if (group.first == defaultGroup) {
            for (const auto& map : group.second) {
                if (map.first == defaultName) {
                    data = map.second.data();
                    break;
                }
            }
        }
    }


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 256, 1, 0, GL_RGB, GL_FLOAT, data);
}

void ColormapModel::applyWrapMode(const std::string& group) {
    // If the group is "cyclic", set OpenGL to repeat the texture
    if (group == "Cyclic") {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    } 
    // Otherwise, clamp to the edge
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

void ColormapModel::selectColormap(const std::string& group, const std::string& name) {
    const float* data = nullptr;
    for (const auto& g : colormaps) {
        if (g.first == group) {
            for (const auto& m : g.second) {
                if (m.first == name) {
                    data = m.second.data();
                    break;
                }
            }
        }
    }
    if (!data) return;

    this->selectedColormapGroup = group;
    this->selectedColormapName = name;
    
    glBindTexture(GL_TEXTURE_2D, *(this->colormapTexture));
    
    // Update the wrap mode dynamically for the new colormap
    applyWrapMode(group);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGB, GL_FLOAT, data);
}
