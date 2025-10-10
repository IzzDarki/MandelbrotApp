/* This file is mostly written by ChatGPT */
#include "screenshot.h"

#include <string>
#include <cstring> // std::memcpyc
#include <iostream>
#include <vector>
#include <cmath>
#include <filesystem>

#include <glad/glad.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" // for writing a png file



// Probably good tiled ChatGPT implementation ------------------------------
bool takeScreenshot(
    std::string filename,
    size_t captureWidth,
    size_t captureHeight,
    Model& model,
    unsigned int vertexArray,
    size_t maxTileSize /* = 2048 */
) {
    // Ensure unique filename: if file exists, append _1, _2, etc.
    if (std::filesystem::exists(filename)) {
        std::string base = filename;
        std::string ext;
        // split extension
        size_t dot = filename.find_last_of('.');
        if (dot != std::string::npos) {
            base = filename.substr(0, dot);
            ext = filename.substr(dot);
        }
        int counter = 1;
        while (std::filesystem::exists(base + "_" + std::to_string(counter) + ext)) {
            ++counter;
        }
        filename = base + "_" + std::to_string(counter) + ext;
    }

    std::cout << "Taking Screenshot \"" << filename << "\" (" << captureWidth << "x" << captureHeight << ")" << std::endl;

    // ---- basic validation -------------------------------------------------
    if (captureWidth == 0 || captureHeight == 0) {
        std::cerr << "    Error: invalid dimensions " << captureWidth << "x" << captureHeight << "\n";
        return false;
    }

    // stbi_write_png takes ints — guard against absurd sizes early.
    if (captureWidth > static_cast<size_t>(std::numeric_limits<int>::max()) ||
        captureHeight > static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        std::cerr << "    Error: requested image dimensions exceed int limits required by PNG writer\n";
        return false;
    }

    // ---- GL limits and tile sizing ---------------------------------------
    GLint maxTexSizeInt = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSizeInt);
    if (maxTexSizeInt <= 0) {
        std::cerr << "    Error: failed to query GL_MAX_TEXTURE_SIZE or returned non-positive value\n";
        return false;
    }
    const size_t maxTexSize = static_cast<size_t>(maxTexSizeInt);

    // Ensure maxTileSize is within what the GL driver supports.
    if (maxTileSize == 0) {
        std::cerr << "    Error: maxTileSize must be > 0\n";
        return false;
    }
    if (maxTileSize > maxTexSize) {
        maxTileSize = maxTexSize;
    }

    // We allow captureWidth/captureHeight > maxTexSize because we render tiled.
    // Calculate how many tiles are required.
    const size_t tilesX = (captureWidth  + maxTileSize - 1) / maxTileSize;
    const size_t tilesY = (captureHeight + maxTileSize - 1) / maxTileSize;

    // actual tile sizes (last tile may be smaller)
    std::vector<size_t> tileWidths(tilesX);
    std::vector<size_t> tileHeights(tilesY);
    for (size_t tx = 0; tx < tilesX; ++tx) {
        tileWidths[tx] = std::min(maxTileSize, captureWidth - tx * maxTileSize);
    }
    for (size_t ty = 0; ty < tilesY; ++ty) {
        tileHeights[ty] = std::min(maxTileSize, captureHeight - ty * maxTileSize);
    }

    // ---- final image buffer (RGBA) --------------------------------------
    const size_t bytesPerPixel = 4u;
    const size_t finalSize = captureWidth * captureHeight * bytesPerPixel;
    std::vector<unsigned char> finalPixels;
    try {
        finalPixels.assign(finalSize, 0u);
    } catch (const std::bad_alloc&) {
        std::cerr << "    Error: not enough memory to allocate final image buffer (" << finalSize << " bytes)\n";
        return false;
    }

    // ---- save & preserve GL state ----------------------------------------
    GLint prevFBO = 0;
    GLint prevViewport[4] = {0,0,0,0};
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    // ---- allocate a single texture + rbo sized to the maximum tile size ----
    GLuint texColor = 0u;
    GLuint rboDepth = 0u;
    GLuint fbo = 0u;

    glGenTextures(1, &texColor);
    glBindTexture(GL_TEXTURE_2D, texColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // allocate storage once for the maximum tile dimensions (safe: <= GL_MAX_TEXTURE_SIZE)
    const int tileTexW = static_cast<int>(maxTileSize);
    const int tileTexH = static_cast<int>(maxTileSize);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tileTexW, tileTexH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // renderbuffer (depth+stencil) sized the same
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, tileTexW, tileTexH);

    // framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // immediate checks
    {
        GLenum e = glGetError();
        if (e != GL_NO_ERROR) {
            std::cerr << "    Error: GL error 0x" << std::hex << e << std::dec
                      << " after allocating tile texture/renderbuffer\n";
            // cleanup before returning
            if (fbo) { glBindFramebuffer(GL_FRAMEBUFFER, 0); glDeleteFramebuffers(1, &fbo); fbo = 0; }
            if (texColor) { glDeleteTextures(1, &texColor); texColor = 0; }
            if (rboDepth) { glDeleteRenderbuffers(1, &rboDepth); rboDepth = 0; }
            // restore viewport/binding
            glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
            glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
            return false;
        }
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "    Error: framebuffer incomplete (0x" << std::hex << status << std::dec << ")\n";
            // cleanup
            glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
            glDeleteFramebuffers(1, &fbo);
            glDeleteTextures(1, &texColor);
            glDeleteRenderbuffers(1, &rboDepth);
            glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
            return false;
        }
    }

    // cleanup helper (restores previous GL state and deletes our temporary objects)
    auto cleanupGL = [&](void) noexcept {
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
        glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
        if (fbo) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
        if (texColor) { glDeleteTextures(1, &texColor); texColor = 0; }
        if (rboDepth) { glDeleteRenderbuffers(1, &rboDepth); rboDepth = 0; }
    };

    bool success = true;

    size_t totalTiles = tilesX * tilesY;
    size_t processedTiles = 0;
    std::cout << "\r    Processed " << processedTiles << "/" << totalTiles << " tiles" << std::flush;

    // ---- render every tile sequentially into the same FBO texture ----------------
    for (size_t ty = 0; ty < tilesY && success; ++ty) {
        for (size_t tx = 0; tx < tilesX; ++tx) {
            const size_t tileW = tileWidths[tx];
            const size_t tileH = tileHeights[ty];

            // set viewport to the *actual* tile size (we render into the lower-left region
            // of the attached texture which is allocated at maxTileSize).
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, static_cast<int>(tileW), static_cast<int>(tileH));
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // set uniforms:
            // - windowSize is the full capture resolution (shader uses gl_FragCoord + tileOffset divided by windowSize)
            // - tileOffset is the pixel offset of this tile in the full image
            model.shader.use();
            // model.shader.setVec2UInt("windowSize", { static_cast<unsigned int>(captureWidth), static_cast<unsigned int>(captureHeight) });
            // model.shader.setVec2Double("center", { static_cast<double>(centerX), static_cast<double>(centerY) });
            // model.shader.setDouble("zoomScale", static_cast<double>(zoomScale));
            const size_t xOffset = tx * maxTileSize;
            const size_t yOffset = ty * maxTileSize;
            model.shader.setVec2UInt("tileOffset", { static_cast<unsigned int>(xOffset), static_cast<unsigned int>(yOffset) });

            // draw
            glBindVertexArray(vertexArray);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
            glFinish();

            // check for GL draw errors
            {
                GLenum err = glGetError();
                if (err != GL_NO_ERROR) {
                    std::cerr << "    Error: GL error 0x" << std::hex << err << std::dec
                              << " while drawing tile (" << tx << "," << ty << ")\n";
                    success = false;
                    break;
                }
            }

            // read pixels for this tile
            const size_t tilePixelCount = tileW * tileH;
            std::vector<unsigned char> tilePixels;
            try {
                tilePixels.resize(tilePixelCount * bytesPerPixel);
            } catch (const std::bad_alloc&) {
                std::cerr << "    Error: out of memory while allocating tile buffer for tile ("
                          << tx << "," << ty << ")\n";
                success = false;
                break;
            }

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glReadPixels(0, 0, static_cast<int>(tileW), static_cast<int>(tileH), GL_RGBA, GL_UNSIGNED_BYTE, tilePixels.data());

            // check glReadPixels
            {
                GLenum err = glGetError();
                if (err != GL_NO_ERROR) {
                    std::cerr << "    Error: GL error 0x" << std::hex << err << std::dec
                              << " after glReadPixels for tile (" << tx << "," << ty << ")\n";
                    success = false;
                    break;
                }
            }

            // copy tile rows into final image buffer with vertical flip
            const size_t xStart = xOffset; // beginning column in final image for this tile
            for (size_t row = 0; row < tileH; ++row) {
                // src - row in tile (bottom-to-top coming from glReadPixels)
                const size_t srcOffset = (row * tileW) * bytesPerPixel;
                // dst - compute row index in final image (flip vertically)
                const size_t globalY = ty * maxTileSize + row; // 0 = top of image? careful: we flip below
                // After flip: destination row index (counting from top=0) is (captureHeight - 1 - globalY)
                const size_t dstRowIndex = (captureHeight - 1 - globalY);
                const size_t dstOffset = (dstRowIndex * captureWidth + xStart) * bytesPerPixel;

                // memcpy the actual tileW bytes-per-row
                std::memcpy(finalPixels.data() + dstOffset, tilePixels.data() + srcOffset, tileW * bytesPerPixel);
            }

            // Update progress
            ++processedTiles;
            std::cout << "\r    Processed " << processedTiles << "/" << totalTiles << " tiles" << std::flush;
        }
    }
    std::cout << "\n"
        << "    Start saving file" << std::endl;

    // ---- write out PNG -----------------------------------------------------
    if (success) {
        // Ensure parent directory exists
        std::filesystem::path outPath(filename);
        if (outPath.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(outPath.parent_path(), ec);
            if (ec) {
                std::cerr << "    Error: failed to create parent directories for "
                            << filename << " (" << ec.message() << ")\n";
                return false;
            }
        }

        // stride in bytes per row — safe, we've checked bounds earlier
        const int rowBytes = static_cast<int>(captureWidth * bytesPerPixel);
        if (stbi_write_png(filename.c_str(),
                           static_cast<int>(captureWidth),
                           static_cast<int>(captureHeight),
                           4,
                           finalPixels.data(),
                           rowBytes) == 0)
        {
            std::cerr << "    Error: failed to write PNG '" << filename << "'\n";
            success = false;
        }
        else {
            std::cout << "    File was saved successfully" << std::endl;
        }
    }

    // cleanup and restore GL state
    cleanupGL();
    return success;
}



// // Probably good ChatGPT Implementation -------------------------------------
// // Save a screenshot of the current scene to `filename` with the requested resolution.
// // Returns true on success.
// bool takeScreenshot(
//     const std::string& filename,
//     int captureWidth,
//     int captureHeight,
//     Shader& shader,
//     unsigned int vertexArray,
//     long double zoomScale,
//     long double realPartStart,
//     long double imagPartStart,
//     int maxSteps
// ) {
//     if (captureWidth <= 0 || captureHeight <= 0) {
//         std::cerr << "takeScreenshot: invalid dimensions " << captureWidth << "x" << captureHeight << "\n";
//         return false;
//     }

//     // Check max texture size
//     GLint maxTexSize = 0;
//     glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
//     if (captureWidth > maxTexSize || captureHeight > maxTexSize) {
//         std::cerr << "takeScreenshot: requested size exceeds GL_MAX_TEXTURE_SIZE (" << maxTexSize << ")\n";
//         return false;
//     }

//     // Save previous GL state
//     GLint prevFBO = 0;
//     GLint prevViewport[4], prevReadBuf = 0, prevDrawBuf = 0;
//     glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);
//     glGetIntegerv(GL_VIEWPORT, prevViewport);
//     glGetIntegerv(GL_READ_BUFFER, &prevReadBuf);
//     glGetIntegerv(GL_DRAW_BUFFER, &prevDrawBuf);

//     // Create FBO + color texture
//     GLuint fbo = 0, texColor = 0, rboDepth = 0;
//     glGenFramebuffers(1, &fbo);
//     glBindFramebuffer(GL_FRAMEBUFFER, fbo);

//     glGenTextures(1, &texColor);
//     glBindTexture(GL_TEXTURE_2D, texColor);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, captureWidth, captureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//     if (glGetError() != GL_NO_ERROR) {
//         std::cerr << "takeScreenshot: failed to allocate color texture\n";
//         glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//         glDeleteFramebuffers(1, &fbo);
//         glDeleteTextures(1, &texColor);
//         return false;
//     }

//     // Optional depth buffer (safe for shaders that use depth)
//     glGenRenderbuffers(1, &rboDepth);
//     glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
//     glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, captureWidth, captureHeight);

//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);
//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

//     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//         std::cerr << "takeScreenshot: framebuffer not complete\n";
//         glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//         glDeleteFramebuffers(1, &fbo);
//         glDeleteTextures(1, &texColor);
//         glDeleteRenderbuffers(1, &rboDepth);
//         return false;
//     }

//     // Set draw target + viewport
//     glDrawBuffer(GL_COLOR_ATTACHMENT0);
//     glViewport(0, 0, captureWidth, captureHeight);

//     // Clear and render
//     glClearColor(0.0f, 0.05f, 0.1f, 1.0f);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//     shader.use();
//     shader.setVec2UInt("windowSize", { (unsigned)captureWidth, (unsigned)captureHeight });
//     shader.setDouble("zoomScale", (double)zoomScale);
//     shader.setVec2Double("numberStart", { realPartStart, imagPartStart });
//     shader.setUInt("MAX_STEPS", (unsigned)maxSteps);

//     glBindVertexArray(vertexArray);
//     glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

//     glFinish(); // Ensure rendering completed

//     // Read pixels
//     const size_t rowBytes = static_cast<size_t>(captureWidth) * 4;
//     std::vector<unsigned char> pixels((size_t)captureHeight * rowBytes);
//     glPixelStorei(GL_PACK_ALIGNMENT, 1);
//     glReadBuffer(GL_COLOR_ATTACHMENT0);
//     glReadPixels(0, 0, captureWidth, captureHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

//     // In-place vertical flip (saves memory vs. copy)
//     for (int y = 0; y < captureHeight / 2; ++y) {
//         unsigned char* row = pixels.data() + (size_t)y * rowBytes;
//         unsigned char* opposite = pixels.data() + (size_t)(captureHeight - 1 - y) * rowBytes;
//         for (size_t i = 0; i < rowBytes; ++i) {
//             std::swap(row[i], opposite[i]);
//         }
//     }

//     // Ensure parent directory exists
//     std::filesystem::path outPath(filename);
//     if (outPath.has_parent_path()) {
//         std::error_code ec;
//         std::filesystem::create_directories(outPath.parent_path(), ec);
//         if (ec) {
//             std::cerr << "takeScreenshot: failed to create parent directories for "
//                         << filename << " (" << ec.message() << ")\n";
//             return false;
//         }
//     }

//     // Write PNG
//     if (stbi_write_png(filename.c_str(), captureWidth, captureHeight, 4, pixels.data(), (int)rowBytes) == 0) {
//         std::cerr << "takeScreenshot: failed to write file '" << filename << "'\n";
//         // Cleanup
//         glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//         glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
//         glDrawBuffer(static_cast<GLuint>(prevDrawBuf));
//         glReadBuffer(static_cast<GLuint>(prevReadBuf));
//         glDeleteFramebuffers(1, &fbo);
//         glDeleteTextures(1, &texColor);
//         glDeleteRenderbuffers(1, &rboDepth);
//         return false;
//     }

//     // Cleanup + restore state
//     glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//     glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
//     glDrawBuffer(static_cast<GLuint>(prevDrawBuf));
//     glReadBuffer(static_cast<GLuint>(prevReadBuf));
//     glDeleteFramebuffers(1, &fbo);
//     glDeleteTextures(1, &texColor);
//     glDeleteRenderbuffers(1, &rboDepth);

//     return true;
// }





// // Save a screenshot of the current scene to `filename` with the requested resolution.
// // - filename: path to a .png file (e.g. "out/screenshot.png")
// // - captureWidth / captureHeight: desired output resolution in pixels
// bool takeScreenshot(
//     const std::string& filename,
//     int captureWidth,
//     int captureHeight,
//     Shader& shader,
//     unsigned int vertexArray,
//     long double zoomScale,
//     long double realPartStart,
//     long double imagPartStart,
//     int maxSteps
// ) {
// 	// Basic validation
// 	if (captureWidth <= 0 || captureHeight <= 0) {
// 		std::cerr << "takeScreenshot: invalid dimensions " << captureWidth << "x" << captureHeight << "\n";
// 		return false;
// 	}

// 	// Check maximum texture/renderbuffer size supported by the GL driver.
// 	GLint maxTexSize = 0;
// 	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
// 	if (captureWidth > maxTexSize || captureHeight > maxTexSize) {
// 		std::cerr << "takeScreenshot: requested size exceeds GL_MAX_TEXTURE_SIZE (" << maxTexSize << ")\n";
// 		return false;
// 	}

// 	// Save current GL state that we will restore later.
// 	GLint prevFBO = 0;
// 	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);

// 	GLint prevViewport[4];
// 	glGetIntegerv(GL_VIEWPORT, prevViewport);

// 	// Create color texture
// 	GLuint texColor = 0;
// 	glGenTextures(1, &texColor);
// 	glBindTexture(GL_TEXTURE_2D, texColor);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 	// No repeat needed, clamp is fine
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 	// Allocate storage
// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, captureWidth, captureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//     // Check for GL errors after allocating the texture (helps detect driver allocation failures)
//     GLenum err = glGetError();
//     if (err != GL_NO_ERROR) {
//         std::cerr << "takeScreenshot: glTexImage2D failed with GL error 0x" << std::hex << err << std::dec << "\n";
//         // cleanup the texture object we created
//         glBindTexture(GL_TEXTURE_2D, 0);
//         glDeleteTextures(1, &texColor);
//         return false;
//     }

// 	// Create depth renderbuffer (some shaders or GL state may require depth)
// 	GLuint rboDepth = 0;
// 	glGenRenderbuffers(1, &rboDepth);
// 	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
// 	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, captureWidth, captureHeight);

// 	// Create framebuffer and attach
// 	GLuint fbo = 0;
// 	glGenFramebuffers(1, &fbo);
// 	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
// 	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);
// 	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

// 	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
// 		std::cerr << "takeScreenshot: framebuffer not complete\n";
// 		// cleanup
// 		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
// 		glDeleteFramebuffers(1, &fbo);
// 		glDeleteTextures(1, &texColor);
// 		glDeleteRenderbuffers(1, &rboDepth);
// 		return false;
// 	}

// 	// Set the draw buffer and viewport for the offscreen target
// 	GLenum drawBuf = GL_COLOR_ATTACHMENT0;
// 	glDrawBuffers(1, &drawBuf);
// 	glViewport(0, 0, captureWidth, captureHeight);

// 	// Clear the offscreen framebuffer
// 	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

// 	// Render the scene into the FBO
// 	// NOTE: we use the existing shader and VAO. Ensure shader uniforms get updated for the capture resolution.
// 	shader.use();
// 	// Set shader uniforms to the capture resolution (these are the things that are set in every iteration of the main loop)
// 	shader.setVec2UInt("windowSize", { static_cast<unsigned int>(captureWidth), static_cast<unsigned int>(captureHeight) });
// 	shader.setDouble("zoomScale", static_cast<double>(zoomScale));
// 	shader.setVec2Double("numberStart", { realPartStart, imagPartStart });
// 	shader.setUInt("MAX_STEPS", static_cast<uint>(maxSteps));

// 	// Draw fullscreen quad
// 	glBindVertexArray(vertexArray);
// 	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

// 	// Make sure all commands have finished
// 	glFinish();

// 	// Read pixels from the FBO
// 	const size_t pixelCount = static_cast<size_t>(captureWidth) * static_cast<size_t>(captureHeight);
// 	const size_t bytesPerPixel = 4u;
// 	std::vector<unsigned char> pixels(pixelCount * bytesPerPixel);

// 	// Pack alignment to 1 to avoid row padding
// 	glPixelStorei(GL_PACK_ALIGNMENT, 1);
// 	glReadBuffer(GL_COLOR_ATTACHMENT0);
// 	glReadPixels(0, 0, captureWidth, captureHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

// 	// Flip vertically because OpenGL's origin is bottom-left while PNG expects top-left
// 	std::vector<unsigned char> flipped(pixels.size());
// 	const size_t rowBytes = static_cast<size_t>(captureWidth) * bytesPerPixel;
// 	for (int y = 0; y < captureHeight; ++y) {
// 		const unsigned char* src = pixels.data() + static_cast<size_t>(y) * rowBytes;
// 		unsigned char* dst = flipped.data() + static_cast<size_t>(captureHeight - 1 - y) * rowBytes;
// 		std::memcpy(dst, src, rowBytes);
// 	}

// 	// Ensure directory exists is left to the caller; attempt to write PNG
// 	int writeResult = stbi_write_png(filename.c_str(), captureWidth, captureHeight, 4, flipped.data(), static_cast<int>(rowBytes));
// 	if (writeResult == 0) {
// 		std::cerr << "takeScreenshot: failed to write PNG to " << filename << "\n";
// 		// cleanup
// 		glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
// 		glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
// 		glDeleteFramebuffers(1, &fbo);
// 		glDeleteTextures(1, &texColor);
// 		glDeleteRenderbuffers(1, &rboDepth);
// 		return false;
// 	}

// 	// cleanup and restore previous GL state
// 	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
// 	glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

// 	glDeleteFramebuffers(1, &fbo);
// 	glDeleteTextures(1, &texColor);
// 	glDeleteRenderbuffers(1, &rboDepth);

// 	return true;
// }

// // Helper: ceil div
// static inline int ceil_div(int a, int b) {
//     return (a + b - 1) / b;
// }

// // Tiled screenshot function
// // - filename: output PNG path
// // - fullWidth, fullHeight: desired final image resolution
// // - preferredTileSize: desired tile size (will be clamped to GL_MAX_TEXTURE_SIZE). Using e.g. 4096 is a reasonable default.
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
//     int preferredTileSize = 1000
// ) {
//     if (fullWidth <= 0 || fullHeight <= 0) {
//         std::cerr << "takeScreenshotTiled: invalid dimensions " << fullWidth << "x" << fullHeight << "\n";
//         return false;
//     }

//     // Query max texture/renderbuffer size
//     GLint maxTexSize = 0;
//     glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
//     GLint maxRbSize = 0;
//     glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRbSize);
//     if (maxTexSize <= 0) {
//         std::cerr << "takeScreenshotTiled: GL_MAX_TEXTURE_SIZE query failed\n";
//         return false;
//     }

//     // Decide tile size (square tiles). Clamp by both MAX_TEXTURE_SIZE and MAX_RENDERBUFFER_SIZE
//     int tileSize = std::min(preferredTileSize, static_cast<int>(maxTexSize));
//     tileSize = std::min(tileSize, static_cast<int>(maxRbSize));

//     if (tileSize <= 0) {
//         std::cerr << "takeScreenshotTiled: computed invalid tile size\n";
//         return false;
//     }

//     // If the user requested something larger than maxTexSize, we still proceed using tiles (that's the point).
//     // Compute number of tiles
//     int tilesX = ceil_div(fullWidth, tileSize);
//     int tilesY = ceil_div(fullHeight, tileSize);

//     // allocate final image buffer (top-left origin rows for stbi_write_png)
//     const size_t fullPixelCount = static_cast<size_t>(fullWidth) * static_cast<size_t>(fullHeight);
//     const size_t bytesPerPixel = 4;
//     std::vector<unsigned char> finalImage(fullPixelCount * bytesPerPixel);
//     std::fill(finalImage.begin(), finalImage.end(), 0); // zero initialize

//     // Save previous GL state so we can restore
//     GLint prevFBO = 0;
//     glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFBO);

//     GLint prevViewport[4] = {0,0,0,0};
//     glGetIntegerv(GL_VIEWPORT, prevViewport);

//     // For each tile create an FBO-sized texture, render, read, copy, cleanup
//     for (int ty = 0; ty < tilesY; ++ty) {
//         for (int tx = 0; tx < tilesX; ++tx) {
//             // tile origin in pixels (x from left, y from bottom)
//             int originX = tx * tileSize;
//             int originY = ty * tileSize;

//             int curW = std::min(tileSize, fullWidth - originX);
//             int curH = std::min(tileSize, fullHeight - originY);

//             // Create color texture for this tile
//             GLuint texColor = 0u;
//             glGenTextures(1, &texColor);
//             glBindTexture(GL_TEXTURE_2D, texColor);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, curW, curH, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

//             GLenum err = glGetError();
//             if (err != GL_NO_ERROR) {
//                 std::cerr << "takeScreenshotTiled: glTexImage2D failed for tile " << tx << "," << ty
//                           << " with GL error 0x" << std::hex << err << std::dec << ".\n";
//                 glDeleteTextures(1, &texColor);
//                 // restore previous FBO/viewport
//                 glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//                 glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
//                 return false;
//             }

//             // Create depth/stencil renderbuffer
//             GLuint rbo = 0u;
//             glGenRenderbuffers(1, &rbo);
//             glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//             glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, curW, curH);

//             // Create framebuffer and attach
//             GLuint fbo = 0u;
//             glGenFramebuffers(1, &fbo);
//             glBindFramebuffer(GL_FRAMEBUFFER, fbo);
//             glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0);
//             glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
//             GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
//             if (status != GL_FRAMEBUFFER_COMPLETE) {
//                 std::cerr << "takeScreenshotTiled: framebuffer incomplete for tile " << tx << "," << ty
//                           << " status 0x" << std::hex << status << std::dec << "\n";
//                 // cleanup
//                 glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//                 glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
//                 glDeleteFramebuffers(1, &fbo);
//                 glDeleteTextures(1, &texColor);
//                 glDeleteRenderbuffers(1, &rbo);
//                 return false;
//             }

//             // Set viewport to tile size
//             glViewport(0, 0, curW, curH);

//             // Clear
//             glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//             glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//             // Render scene for this tile.
//             shader.use();

//             // IMPORTANT uniform setup for tiling:
//             // - tileOffset: pixel offset of this tile relative to the lower-left corner of the full image
//             // The shader must use (tileOffset + gl_FragCoord.xy) as the "global pixel index" inside the mapping.
//             shader.setVec2UInt("tileOffset", { static_cast<unsigned int>(originX), static_cast<unsigned int>(originY) });

//             // Also set any other uniforms you normally set (zoom, numberStart, MAX_STEPS, etc.)
//             // KEEP these consistent with your normal rendering:
//             shader.setDouble("zoomScale", static_cast<double>(zoomScale)); // uses your global zoomScale
//             shader.setVec2Double("numberStart", { realPartStart, imagPartStart }); // your global start
//             shader.setVec2UInt("windowSize", { static_cast<unsigned int>(fullWidth), static_cast<unsigned int>(fullHeight) });
//             shader.setUInt("MAX_STEPS", static_cast<unsigned int>(maxSteps));

//             std::cout << "Tile Offset: (" << originX << ", " << originY << "), size: (" << fullWidth << ", " << fullHeight << ")" << std::endl;

//             // Draw fullscreen quad (the shader uses tileOffset + gl_FragCoord to map correctly)
//             glBindVertexArray(vertexArray);
//             glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

//             glFinish(); // ensure rendering finished

//             // Read pixels from the tile (GL origin is bottom-left)
//             std::vector<unsigned char> tilePixels(static_cast<size_t>(curW) * static_cast<size_t>(curH) * bytesPerPixel);
//             glPixelStorei(GL_PACK_ALIGNMENT, 1);
//             glReadBuffer(GL_COLOR_ATTACHMENT0);
//             glReadPixels(0, 0, curW, curH, GL_RGBA, GL_UNSIGNED_BYTE, tilePixels.data());

//             GLenum readErr = glGetError();
//             if (readErr != GL_NO_ERROR) {
//                 std::cerr << "takeScreenshotTiled: glReadPixels error 0x" << std::hex << readErr << std::dec << " for tile " << tx << "," << ty << "\n";
//                 // cleanup
//                 glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//                 glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
//                 glDeleteFramebuffers(1, &fbo);
//                 glDeleteTextures(1, &texColor);
//                 glDeleteRenderbuffers(1, &rbo);
//                 return false;
//             }

//             // Stitch tilePixels into finalImage.
//             // tilePixels row 0 is bottom row of this tile. finalImage expects top-first rows.
//             for (int row = 0; row < curH; ++row) {
//                 // global Y coordinate of this tile row (0 = bottom)
//                 int globalY = originY + row;
//                 // destination row index in finalImage where 0 is top row
//                 int destRow = (fullHeight - 1) - globalY;
//                 unsigned char* destPtr = finalImage.data() + (static_cast<size_t>(destRow) * static_cast<size_t>(fullWidth) + static_cast<size_t>(originX)) * bytesPerPixel;
//                 unsigned char* srcPtr  = tilePixels.data() + (static_cast<size_t>(row) * static_cast<size_t>(curW)) * bytesPerPixel;
//                 // copy curW * 4 bytes
//                 std::memcpy(destPtr, srcPtr, static_cast<size_t>(curW) * bytesPerPixel);
//             }

//             // cleanup this tile's GL objects
//             glBindFramebuffer(GL_FRAMEBUFFER, 0);
//             glDeleteFramebuffers(1, &fbo);
//             glDeleteTextures(1, &texColor);
//             glDeleteRenderbuffers(1, &rbo);
//         } // tx
//     } // ty

//     // Restore previous GL state
//     glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFBO));
//     glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);

//     // Write finalImage to PNG
//     int stride = fullWidth * static_cast<int>(bytesPerPixel);
//     if (!stbi_write_png(filename.c_str(), fullWidth, fullHeight, 4, finalImage.data(), stride)) {
//         std::cerr << "takeScreenshotTiled: failed to write PNG to " << filename << "\n";
//         return false;
//     }

//     return true;
// }
