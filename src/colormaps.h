#pragma once
#ifndef MANDELBROT_COLORMAP_INCLUDED
#define MANDELBROT_COLORMAP_INCLUDED

#include <string>
#include <vector>

using ColormapGroup = std::pair<std::string, std::vector<std::pair<std::string, std::vector<float>>>>;
using ColormapDictionary = std::vector<ColormapGroup>;
inline ColormapDictionary colormaps;

void loadColormaps(const std::string& filepath);

#endif
