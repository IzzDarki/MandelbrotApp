#pragma once
#ifndef MANDELBROT_SHADER_INCLUDED
#define MANDELBROT_SHADER_INCLUDED

#include "app_utility.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include <glad/glad.h>

class Shader
{
protected:

    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;

    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::unordered_map<std::string, std::string> defines;

public:
    Shader() = default;
    /**
     * Creates a Shader
     * 
     * @param vertexShaderSource Vertex shader source
     * @param fragmentShaderSource Fragment shader source
     * @param compileAndLink When `true` the shader sources will be compiled and linked instantly, otherwise this can be done manually later (default is `true`)
     * @param clean When `true` the openGL shaders and the shader sources will be deleted after compiling and linking (default is `true`)
     */
    Shader(const std::string& vertexShaderSourcePath, const std::string& fragmentShaderSourcePath, bool compileAndLink = true, bool clean = true);

    void compileVertexShader();
    void compileFragmentShader();
    void link();
    void use() const;
    inline void deleteVertexShader() { glDeleteShader(vertexShader);}
    inline void deleteFragmentShader() { glDeleteShader(fragmentShader);}
    void deleteShaders();
    void deleteProgram();

    /**
     * Deletes openGL shaders, shader sources and defines
     */
    void clean();

    void setInt(const std::string& name, int value);
    void setVec2Int(const std::string& name, int x, int y);
    void setVec3Int(const std::string& name, int x, int y, int z);
    void setVec4Int(const std::string& name, int x, int y, int z, int w);
    void setUInt(const std::string& name, unsigned int value);
    void setVec2UInt(const std::string& name, unsigned int x, unsigned int y);
    void setVec3UInt(const std::string& name, unsigned int x, unsigned int y, unsigned int z);
    void setVec4UInt(const std::string& name, unsigned int x, unsigned int y, unsigned int z, unsigned int w);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, float x, float y);
    void setVec3(const std::string& name, float x, float y, float z);
    void setVec4(const std::string& name, float x, float y, float z, float w);
    void setDouble(const std::string& name, double value);
    void setVec2Double(const std::string& name, double x, double y);
    void setVec3Double(const std::string& name, double x, double y, double z);
    void setVec4Double(const std::string& name, double x, double y, double z, double w);

    /**
     * When later compiling the the shaders, every occurrence of `name` in the all the shader sources will be replaced by `value`
     * 
     * @param name The string that gets replaced
     * @param value The string to replace with
     */
    inline void define(const std::string& name, const std::string& value) { defines[name] = value; }

    void mandelRecompileWithColor(int colorNumber);

protected: // helpers

    std::string replaceDefines(const std::string& shaderSource) const;

    /** 
     * @param type Needs to be either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * @return Id of the shader object
     */
    static unsigned int loadShaderFromFile(int type, const std::string& shaderSource);

    /**
     * @return Id of the program object 
     */
    static unsigned int linkShaderProgram(unsigned int vertexShader, unsigned int fragmentShader);

};

#endif
