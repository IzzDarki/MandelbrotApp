#include "app_utility.h"

#include <cmath>
#include <format>

#include <ImGui/imgui.h>

void copyStringToBuffer(const std::string& string, char* buffer, unsigned int size) {
    auto index = string.copy(buffer, size - 1);
    buffer[index] = '\0';
}

int hashDjb2(const std::string& str) {
    int hash = 5381;
    for (auto c : str) {
        hash = (hash << 5) + hash + static_cast<int>(c); /* hash * 33 + c */
    }
    return hash;
}

bool replaceAll(std::string& string, const std::string& search, const std::string& replace) {
    std::string::size_type pos = 0;
    while ((pos = string.find(search, pos)) != std::string::npos) {
        string.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    
    return (pos != 0);
}

std::string readFileToString(const char* filePath) {
    std::ifstream inputStream(filePath);
    //inputStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    std::stringstream stringStream; 
    stringStream << inputStream.rdbuf();
    return stringStream.str();
}


std::string getDirectoryFromFilePath(const std::string& filePath) {
    auto pos = filePath.rfind('/');

    if (pos == std::string::npos) {
        return "";
    } else {
        return filePath.substr(0, pos + 1);
    }
}

bool ImGuiFlexibleSliderInt(
    const char* label,
    int* value,
    int* minVal,
    int* maxVal,
    float ratioLower,
    float ratioUpper)
{
    bool changed = false;

    ImGui::BeginGroup();

    // Draw label on its own line
    ImGui::TextUnformatted(label);
    ImGui::PushID(label);

    // const float sliderWidth = ImGui::GetContentRegionAvail().x;
    // const float buttonWidth = ImGui::GetFrameHeight(); // small square buttons

    // Optional left-side scaling buttons
    if (ratioLower != 1.0f) {
        if (ImGui::SmallButton("-##lower")) {
            *minVal = std::min(*maxVal, static_cast<int>(std::floor(static_cast<float>(*minVal) / ratioLower)));
            if (*value < *minVal) {
                *value = *minVal;
                changed = true;
            }
            //changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+##lower")) {
            *minVal = std::min(*maxVal, static_cast<int>(std::ceil(static_cast<float>(*minVal) * ratioLower)));
            if (*value < *minVal) {
                *value = *minVal;
                changed = true;
            }
            //changed = true;
        }
        ImGui::SameLine();
    }

    // Slider in the middle
    // float effectiveWidth = sliderWidth;
    // if (ratioLower != 1.0f) effectiveWidth -= (buttonWidth * 2.5f);
    // if (ratioUpper != 1.0f) effectiveWidth -= (buttonWidth * 2.5f);

    // ImGui::SetNextItemWidth(effectiveWidth);
    changed |= ImGui::SliderInt("##slider", value, *minVal, *maxVal);

    // Optional right-side scaling buttons
    if (ratioUpper != 1.0f) {
        ImGui::SameLine();
        if (ImGui::SmallButton("-##upper")) {
            *maxVal = std::max(*minVal, static_cast<int>(std::floor(static_cast<float>(*maxVal) / ratioUpper)));
            if (*value > *maxVal) {
                *value = *maxVal;
                changed = true;
            }
            //changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+##upper")) {
            *maxVal = std::max(*minVal, static_cast<int>(std::ceil(static_cast<float>(*maxVal) * ratioUpper)));
            if (*value > *maxVal) {
                *value = *maxVal;
                changed = true;
            }
            //changed = true;
        }
    }

    ImGui::PopID();
    ImGui::EndGroup();

    return changed;
}

bool ImGuiFlexibleSliderFloat(
    const char* label,
    float* value,
    float* minVal,
    float* maxVal,
    float ratioLower,
    float ratioUpper)
{
    bool changed = false;

    ImGui::BeginGroup();

    ImGui::TextUnformatted(label);
    ImGui::PushID(label);

    // const float sliderWidth = ImGui::GetContentRegionAvail().x;
    // const float buttonWidth = ImGui::GetFrameHeight(); // small square buttons

    // Optional left-side scaling buttons
    if (ratioLower != 1.0f) {
        if (ImGui::SmallButton("-##lower")) {
            *minVal = std::min(*maxVal, *minVal / ratioLower);
            if (*value < *minVal) {
                *value = *minVal;
                changed = true;
            }
            //changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+##lower")) {
            *minVal = std::min(*maxVal, *minVal * ratioLower);
            if (*value < *minVal) {
                *value = *minVal;
                changed = true;
            }
            //changed = true;
        }
        ImGui::SameLine();
    }

    // Slider in the middle
    // float effectiveWidth = sliderWidth;
    // if (ratioLower != 1.0f) effectiveWidth -= (buttonWidth * 2.5f);
    // if (ratioUpper != 1.0f) effectiveWidth -= (buttonWidth * 2.5f);

    // ImGui::SetNextItemWidth(effectiveWidth);
    changed |= ImGui::SliderFloat("##slider", value, *minVal, *maxVal);

    // Optional right-side scaling buttons
    if (ratioUpper != 1.0f) {
        ImGui::SameLine();
        if (ImGui::SmallButton("-##upper")) {
            *maxVal = std::max(*minVal, *maxVal / ratioUpper);
            if (*value > *maxVal) {
                *value = *maxVal;
                changed = true;
            }
            //changed = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+##upper")) {
            *maxVal = std::max(*minVal, *maxVal * ratioUpper);
            if (*value > *maxVal) {
                *value = *maxVal;
                changed = true;
            }
            //changed = true;
        }
    }

    ImGui::PopID();

    ImGui::EndGroup();
    return changed;
}