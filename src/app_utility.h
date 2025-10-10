#pragma once
#ifndef MANDELBROT_APPUTILITY_INCLUDED
#define MANDELBROT_APPUTILITY_INCLUDED

#include <string>
#include <fstream>
#include <sstream>

/**
 * Type representing a complex number
 * `first` is the real part
 * `second` is the imaginary part
 */
using ComplexNum = std::pair<long double, long double>;

/**
 * Copies a string to a buffer
 * 
 * @param string String to copy from
 * @param buffer `char*` buffer to copy to, will be null-terminated
 * @param size The maximum number of characters to copy (including null-terminating character) (= max size of the buffer)
 */
void copyStringToBuffer(const std::string& string, char* buffer, unsigned int size);

/**
 * Hash algorithm for strings (djb2)
 */
int hashDjb2(const std::string& str);

/**
 * Replaces all occurrences of `search` in `string` with `replace`
 * 
 * @param string String to modify
 * @param search String search for
 * @param replace String to replace with
 * @return Returns `true` if `string` was modified, `false` otherwise
 */
bool replaceAll(std::string& string, const std::string& search, const std::string& replace);

/**
 * Reads file to a string
 * 
 * @param filePath Path of the file
 * @return Returns the contents of the file
 * @throws Could throw `std::ifstream::failure`
 */
std::string readFileToString(const char* filePath);

/** 
 * @param filePath e.g. "res/shader.glsl"
 * @return std::string e.g. "res/"
 */
std::string getDirectoryFromFilePath(const std::string& filePath);

/** 
 * By ChatGPT
 * A reusable ImGui helper for a slider with dynamic bounds and optional scaling buttons
 * ratioLower and ratioUpper control how much the bounds change when +/- buttons are pressed.
 * If either is 1.0f, that side's buttons are not shown.
 */
bool ImGuiFlexibleSliderInt(
    const char* label,
    int* value,
    int* minVal,
    int* maxVal,
    float ratioLower = 1.0f,
    float ratioUpper = 1.0f);

/** 
 * By ChatGPT
 * A reusable ImGui helper for a slider with dynamic bounds and optional scaling buttons
 * ratioLower and ratioUpper control how much the bounds change when +/- buttons are pressed.
 * If either is 1.0f, that side's buttons are not shown.
 */
bool ImGuiFlexibleSliderFloat(
    const char* label,
    float* value,
    float* minVal,
    float* maxVal,
    float ratioLower = 1.0f,
    float ratioUpper = 1.0f);

#endif
