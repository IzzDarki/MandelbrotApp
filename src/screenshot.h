#pragma once
#include <string>

bool takeScreenshot(
    std::string filename,
    size_t captureWidth,
    size_t captureHeight,
    Shader& shader,
    unsigned int vertexArray,
    long double zoomScale,
    long double realPartStart,
    long double imagPartStart,
    int maxSteps,
    size_t maxTileSize = 2048 // maximum tile width/height in pixels
);

// bool takeScreenshotTiled(
//     const std::string& filename,
//     int fullWidth,
//     int fullHeight,
//     Shader& shader,
//     unsigned int vertexArray,
//     long double zoomScale,
//     long double realPartStart,
//     long double imagPartStart,
//     int maxSteps,
//     int preferredTileSize = 1920
// );
