#pragma once
#ifndef MANDELBROT_SHADER_INCLUDED
#define MANDELBROT_SHADER_INCLUDED

#include "app_utility.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <tuple>
#include <vector>

#include <glad/glad.h>

namespace vec {
    using vec2int = std::tuple<int, int>;
    using vec3int = std::tuple<int, int, int>;
    using vec4int = std::tuple<int, int, int, int>;
    using vec2uint = std::tuple<uint, uint>;
    using vec3uint = std::tuple<uint, uint, uint>;
    using vec4uint = std::tuple<uint, uint, uint, uint>;
    using vec2 = std::tuple<float, float>;
    using vec3 = std::tuple<float, float, float>;
    using vec4 = std::tuple<float, float, float, float>;
    using vec2double = std::tuple<double, double>;
    using vec3double = std::tuple<double, double, double>;
    using vec4double = std::tuple<double, double, double, double>;

    template <typename... Ts>
    static decltype(auto) getX(const std::tuple<Ts...>& tuple) {
        return std::get<0>(tuple);
    }

    template <typename... Ts>
    static decltype(auto) getY(const std::tuple<Ts...>& tuple) {
        return std::get<1>(tuple);
    }

    template <typename... Ts>
    static decltype(auto) getZ(const std::tuple<Ts...>& tuple) {
        return std::get<2>(tuple);
    }

    template <typename... Ts>
    static decltype(auto) getW(const std::tuple<Ts...>& tuple) {
        return std::get<3>(tuple);
    }
}

class Shader
{
public:
    using vec2int = vec::vec2int;
    using vec3int = vec::vec3int;
    using vec4int = vec::vec4int;
    using vec2uint = vec::vec2uint;
    using vec3uint = vec::vec3uint;
    using vec4uint = vec::vec4uint;
    using vec2 = vec::vec2;
    using vec3 = vec::vec3;
    using vec4 = vec::vec4;
    using vec2double = vec::vec2double;
    using vec3double = vec::vec3double;
    using vec4double = vec::vec4double;
    
    using uniform_t = std::variant<
        int, vec2int, vec3int, vec4int,
        uint, vec2uint, vec3uint, vec4uint,
        float, vec2, vec3, vec4,
        double, vec2double, vec3double, vec4double,
        std::vector<int>, std::vector<vec2int>, std::vector<vec3int>, std::vector<vec4int>,
        std::vector<uint>, std::vector<vec2uint>, std::vector<vec3uint>, std::vector<vec4uint>,
        std::vector<float>, std::vector<vec2>, std::vector<vec3>, std::vector<vec4>,

        std::vector<double>, std::vector<vec2double>, std::vector<vec3double>, std::vector<vec4double>
        >;

public:
    Shader() = default;
    Shader(const Shader& other);
    Shader(Shader&& other) noexcept;
    /**
     * Creates a Shader
     * 
     * @param vertexShaderSource Vertex shader source
     * @param fragmentShaderSource Fragment shader source
     * @param compileAndLink When `true` the shader sources will be compiled and linked instantly, otherwise this can be done manually later (default is `true`)
     */
    Shader(const std::string& vertexShaderSourcePath, const std::string& fragmentShaderSourcePath, bool compileAndLink = true);
    ~Shader();

    Shader& operator=(const Shader& other);
    Shader& operator=(Shader&& other) noexcept;

    void compileVertexShader();
    void compileFragmentShader();
    void link();
    inline void compileAndLink() { compileVertexShader(); compileFragmentShader(); link(); }
    void use() const;
    void deleteVertexShader();
    void deleteFragmentShader();
    void destroy();
    void deleteShaders();
    void deleteProgram();

    void setInt(const std::string& name, int val);
    void setVec2Int(const std::string& name, vec2int val);
    void setVec3Int(const std::string& name, vec3int val);
    void setVec4Int(const std::string& name, vec4int val);
    void setUInt(const std::string& name, uint val);
    void setVec2UInt(const std::string& name, vec2uint val);
    void setVec3UInt(const std::string& name, vec3uint val);
    void setVec4UInt(const std::string& name, vec4uint val);
    void setFloat(const std::string& name, float val);
    void setVec2(const std::string& name, vec2 val);
    void setVec3(const std::string& name, vec3 val);
    void setVec4(const std::string& name, vec4 val);
    void setDouble(const std::string& name, double val);
    void setVec2Double(const std::string& name, vec2double val);
    void setVec3Double(const std::string& name, vec3double val);
    void setVec4Double(const std::string& name, vec4double val);
    void setIntArray(const std::string& name, const int* vals, uint count);
    void setVec2IntArray(const std::string& name, const vec2int* vals, uint count);
    void setVec3IntArray(const std::string& name, const vec3int* vals, uint count);
    void setVec4IntArray(const std::string& name, const vec4int* vals, uint count);
    void setUIntArray(const std::string& name, const uint* vals, uint count);
    void setVec2UIntArray(const std::string& name, const vec2uint* vals, uint count);
    void setVec3UIntArray(const std::string& name, const vec3uint* vals, uint count);
    void setVec4UIntArray(const std::string& name, const vec4uint* vals, uint count);
    void setFloatArray(const std::string& name, const float* vals, uint count);
    void setVec2Array(const std::string& name, const vec2* vals, uint count);
    void setVec3Array(const std::string& name, const vec3* vals, uint count);
    void setVec4Array(const std::string& name, const vec4* vals, uint count);
    void setDoubleArray(const std::string& name, const double* vals, uint count);
    void setVec2DoubleArray(const std::string& name, const vec2double* vals, uint count);
    void setVec3DoubleArray(const std::string& name, const vec3double* vals, uint count);
    void setVec4DoubleArray(const std::string& name, const vec4double* vals, uint count);

    auto getInt(const std::string& name) -> int;
    auto getVec2Int(const std::string& name) -> vec2int;
    auto getVec3Int(const std::string& name) -> vec3int;
    auto getVec4Int(const std::string& name) -> vec4int;
    auto getUInt(const std::string& name) -> uint;
    auto getVec2UInt(const std::string& name) -> vec2uint;
    auto getVec3UInt(const std::string& name) -> vec3uint;
    auto getVec4UInt(const std::string& name) -> vec4uint;
    auto getFloat(const std::string& name) -> float;
    auto getVec2(const std::string& name) -> vec2;
    auto getVec3(const std::string& name) -> vec3;
    auto getVec4(const std::string& name) -> vec4;
    auto getDouble(const std::string& name) -> double;
    auto getVec2Double(const std::string& name) -> vec2double;
    auto getVec3Double(const std::string& name) -> vec3double;
    auto getVec4Double(const std::string& name) -> vec4double;
    auto getIntArray(const std::string& name) -> const std::vector<int>&;
    auto getVec2IntArray(const std::string& name) -> const std::vector<vec2int>&;
    auto getVec3IntArray(const std::string& name) -> const std::vector<vec3int>&;
    auto getVec4IntArray(const std::string& name) -> const std::vector<vec4int>&;
    auto getUIntArray(const std::string& name) -> const std::vector<uint>&;
    auto getVec2UIntArray(const std::string& name) -> const std::vector<vec2uint>&;
    auto getVec3UIntArray(const std::string& name) -> const std::vector<vec3uint>&;
    auto getVec4UIntArray(const std::string& name) -> const std::vector<vec4uint>&;
    auto getFloatArray(const std::string& name) -> const std::vector<float>&;
    auto getVec2Array(const std::string& name) -> const std::vector<vec2>&;
    auto getVec3Array(const std::string& name) -> const std::vector<vec3>&;
    auto getVec4Array(const std::string& name) -> const std::vector<vec4>&;
    auto getDoubleArray(const std::string& name) -> const std::vector<double>&;
    auto getVec2DoubleArray(const std::string& name) -> const std::vector<vec2double>&;
    auto getVec3DoubleArray(const std::string& name) -> const std::vector<vec3double>&;
    auto getVec4DoubleArray(const std::string& name) -> const std::vector<vec4double>&;

    /**
     * When later compiling the the shaders, every occurrence of `name` in the all the shader sources will be replaced by `value`
     * 
     * @param name The string that gets replaced
     * @param value The string to replace with
     */
    inline void define(const std::string& name, const std::string& value) { defines[name] = value; }

    void recompile();

protected:

    unsigned int vertexShader = 0; // for OpenGL 0 is "no shader"
    unsigned int fragmentShader = 0; // for OpenGL 0 is "no shader"
    unsigned int shaderProgram = 0; // for OpenGL 0 is "no program"

    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::unordered_map<std::string, std::string> defines;
    std::unordered_map<std::string, uniform_t> uniforms;

protected: // helpers

    std::string prependDefines(const std::string& shaderSource);

    /**
     * Loads shader source code from a path and recursively loads #include dependencies which are literally copy pasted into the source code of the parent shader
     * #includes must be relative to the directory, where the shader file itself is located, i.e. in "res/parent.glsl" includes must be relative to "res/"
    */
    static std::string loadShaderSourceFromPath(const std::string& shaderSourcePath, unsigned int maxDepth = 20, unsigned int depth = 0);

    /** 
     * @param type Needs to be either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
     * @return Id of the shader object
     */
    static unsigned int loadShaderFromSource(GLenum type, const std::string& shaderSource);

    /**
     * @return Id of the program object 
     */
    static unsigned int linkShaderProgram(unsigned int vertexShader, unsigned int fragmentShader);

};

#endif
