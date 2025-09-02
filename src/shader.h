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
        int,
        vec2int,
        vec3int,
        vec4int,
        uint,
        vec2uint,
        vec3uint,
        vec4uint,
        float,
        vec2,
        vec3,
        vec4,
        double,
        vec2double,
        vec3double,
        vec4double>;

    static constexpr auto MANDEL_FLOW_COLOR_TYPE = "FLOW_COLOR_TYPE";

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

    /**
     * When later compiling the the shaders, every occurrence of `name` in the all the shader sources will be replaced by `value`
     * 
     * @param name The string that gets replaced
     * @param value The string to replace with
     */
    inline void define(const std::string& name, const std::string& value) { defines[name] = value; }

    void recompile();

protected:

    unsigned int vertexShader;
    unsigned int fragmentShader;
    unsigned int shaderProgram;

    std::string vertexShaderSource;
    std::string fragmentShaderSource;
    std::unordered_map<std::string, std::string> defines;
    std::unordered_map<std::string, uniform_t> uniforms;

protected: // helpers

    std::string replaceDefines(const std::string& shaderSource) const;

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
