#include "shader.h"

#include <regex>
#include <string>

using namespace vec;

Shader::Shader(const std::string& vertexShaderSourcePath, const std::string& fragmentShaderSourcePath, bool compileAndLink, bool clean) {

    // this->defines = defines;
    vertexShaderSource = Shader::loadShaderSourceFromPath(vertexShaderSourcePath);
    fragmentShaderSource = Shader::loadShaderSourceFromPath(fragmentShaderSourcePath);

    if (compileAndLink) {
        compileVertexShader();
        compileFragmentShader();
        link();
        
        if (clean)
            this->clean();
    }
}

void Shader::clean() {
    deleteShaders();
    vertexShaderSource.clear();
    fragmentShaderSource.clear();
    defines.clear();
}

void Shader::deleteShaders() {
    deleteVertexShader();
    deleteFragmentShader();
}

void Shader::deleteProgram() {
    glDeleteProgram(shaderProgram);
}

void Shader::compileVertexShader() {
    std::string finalShaderSource = replaceDefines(vertexShaderSource);
    vertexShader = loadShaderFromSource(GL_VERTEX_SHADER, finalShaderSource);
}

void Shader::compileFragmentShader() {
    std::string finalShaderSource = replaceDefines(fragmentShaderSource);
    fragmentShader = loadShaderFromSource(GL_FRAGMENT_SHADER, finalShaderSource);
}

void Shader::link() {
    shaderProgram = linkShaderProgram(vertexShader, fragmentShader);
}

void Shader::use() const {
    glUseProgram(shaderProgram);
}


// * Uniform setter helpers
inline void setIntHelper(unsigned int shaderProgram, const std::string& name, int val) { glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), val); }
inline void setVec2IntHelper(unsigned int shaderProgram, const std::string& name, vec2int val) { glUniform2i(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val)); }
inline void setVec3IntHelper(unsigned int shaderProgram, const std::string& name, vec3int val) { glUniform3i(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val)); }
inline void setVec4IntHelper(unsigned int shaderProgram, const std::string& name, vec4int val) { glUniform4i(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val), getW(val)); }
inline void setUIntHelper(unsigned int shaderProgram, const std::string& name, uint val) { glUniform1ui(glGetUniformLocation(shaderProgram, name.c_str()), val); }
inline void setVec2UIntHelper(unsigned int shaderProgram, const std::string& name, vec2uint val) { glUniform2ui(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val)); }
inline void setVec3UIntHelper(unsigned int shaderProgram, const std::string& name, vec3uint val) { glUniform3ui(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val)); }
inline void setVec4UIntHelper(unsigned int shaderProgram, const std::string& name, vec4uint val) { glUniform4ui(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val), getW(val)); }
inline void setFloatHelper(unsigned int shaderProgram, const std::string& name, float val) { glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), val); }
inline void setVec2Helper(unsigned int shaderProgram, const std::string& name, vec2 val) { glUniform2f(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val)); }
inline void setVec3Helper(unsigned int shaderProgram, const std::string& name, vec3 val) { glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val)); }
inline void setVec4Helper(unsigned int shaderProgram, const std::string& name, vec4 val) { glUniform4f(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val), getW(val)); }
inline void setDoubleHelper(unsigned int shaderProgram, const std::string& name, double val) { glUniform1d(glGetUniformLocation(shaderProgram, name.c_str()), val); }
inline void setVec2DoubleHelper(unsigned int shaderProgram, const std::string& name, vec2double val) { glUniform2d(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val)); }
inline void setVec3DoubleHelper(unsigned int shaderProgram, const std::string& name, vec3double val) { glUniform3d(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val)); }
inline void setVec4DoubleHelper(unsigned int shaderProgram, const std::string& name, vec4double val) { glUniform4d(glGetUniformLocation(shaderProgram, name.c_str()), getX(val), getY(val), getZ(val), getW(val)); }

inline void setIntArrayHelper(unsigned int shaderProgram, const std::string& name, const int* vals, uint count) {
    glUniform1iv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec2IntArrayHelper(unsigned int shaderProgram, const std::string& name, const int* vals, uint count) {
    glUniform2iv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec3IntArrayHelper(unsigned int shaderProgram, const std::string& name, const int* vals, uint count) {
    glUniform3iv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec4IntArrayHelper(unsigned int shaderProgram, const std::string& name, const int* vals, uint count) {
    glUniform4iv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setUIntArrayHelper(unsigned int shaderProgram, const std::string& name, const unsigned int* vals, uint count) {
    glUniform1uiv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec2UIntArrayHelper(unsigned int shaderProgram, const std::string& name, const unsigned int* vals, uint count) {
    glUniform2uiv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec3UIntArrayHelper(unsigned int shaderProgram, const std::string& name, const unsigned int* vals, uint count) {
    glUniform3uiv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec4UIntArrayHelper(unsigned int shaderProgram, const std::string& name, const unsigned int* vals, uint count) {
    glUniform4uiv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setFloatArrayHelper(unsigned int shaderProgram, const std::string& name, const float* vals, uint count) {
    glUniform1fv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec2ArrayHelper(unsigned int shaderProgram, const std::string& name, const float* vals, uint count) {
    glUniform2fv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec3ArrayHelper(unsigned int shaderProgram, const std::string& name, const float* vals, uint count) {
    glUniform3fv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec4ArrayHelper(unsigned int shaderProgram, const std::string& name, const float* vals, uint count) {
    glUniform4fv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setDoubleArrayHelper(unsigned int shaderProgram, const std::string& name, const double* vals, uint count) {
    glUniform1dv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec2DoubleArrayHelper(unsigned int shaderProgram, const std::string& name, const double* vals, uint count) {
    glUniform2dv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec3DoubleArrayHelper(unsigned int shaderProgram, const std::string& name, const double* vals, uint count) {
    glUniform3dv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}

inline void setVec4DoubleArrayHelper(unsigned int shaderProgram, const std::string& name, const double* vals, uint count) {
    glUniform4dv(glGetUniformLocation(shaderProgram, name.c_str()), static_cast<GLsizei>(count), vals);
}


// * Uniform Setters

void Shader::setInt(const std::string& name, int val) {
    setIntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec2Int(const std::string& name, vec2int val) {
    setVec2IntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec3Int(const std::string& name, vec3int val) {
    setVec3IntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec4Int(const std::string& name, vec4int val) {
    setVec4IntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setUInt(const std::string& name, uint val) {
    setUIntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec2UInt(const std::string& name, vec2uint val) {
    setVec2UIntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec3UInt(const std::string& name, vec3uint val) {
    setVec3UIntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec4UInt(const std::string& name, vec4uint val) {
    setVec4UIntHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setFloat(const std::string& name, float val) {
    setFloatHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec2(const std::string& name, vec2 val) {
    setVec2Helper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec3(const std::string& name, vec3 val) {
    setVec3Helper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec4(const std::string& name, vec4 val) {
    setVec4Helper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setDouble(const std::string& name, double val) {
    setDoubleHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec2Double(const std::string& name, vec2double val) {
    setVec2DoubleHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec3Double(const std::string& name, vec3double val) {
    setVec3DoubleHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setVec4Double(const std::string& name, vec4double val) {
    setVec4DoubleHelper(shaderProgram, name, val);
    uniforms[name] = val;
}

void Shader::setIntArray(const std::string& name, const int* vals, uint count) {
    uniforms[name] = std::vector<int>(vals, vals+count);
    const auto& vector = this->getIntArray(name);
    setIntArrayHelper(shaderProgram, name, vector.data(), count);
}

void Shader::setVec2IntArray(const std::string& name, const vec2int* vals, uint count) {
    uniforms[name] = std::vector<vec2int>(vals, vals+count);
    const auto& vector = this->getVec2IntArray(name);

    std::vector<int> flat;
    flat.reserve(count * 2);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
    }
    setVec2IntArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec3IntArray(const std::string& name, const vec3int* vals, uint count) {
    uniforms[name] = std::vector<vec3int>(vals, vals+count);
    const auto& vector = this->getVec3IntArray(name);

    std::vector<int> flat;
    flat.reserve(count * 3);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
    }
    setVec3IntArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec4IntArray(const std::string& name, const vec4int* vals, uint count) {
    uniforms[name] = std::vector<vec4int>(vals, vals+count);
    const auto& vector = this->getVec4IntArray(name);

    std::vector<int> flat;
    flat.reserve(count * 4);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
        flat.push_back(std::get<3>(vec));
    }
    setVec4IntArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setUIntArray(const std::string& name, const uint* vals, uint count) {
    uniforms[name] = std::vector<uint>(vals, vals+count);
    const auto& vector = this->getUIntArray(name);
    setUIntArrayHelper(shaderProgram, name, vector.data(), count);
}

void Shader::setVec2UIntArray(const std::string& name, const vec2uint* vals, uint count) {
    uniforms[name] = std::vector<vec2uint>(vals, vals+count);
    const auto& vector = this->getVec2UIntArray(name);

    std::vector<uint> flat;
    flat.reserve(count * 2);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
    }
    setVec2UIntArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec3UIntArray(const std::string& name, const vec3uint* vals, uint count) {
    uniforms[name] = std::vector<vec3uint>(vals, vals+count);
    const auto& vector = this->getVec3UIntArray(name);

    std::vector<uint> flat;
    flat.reserve(count * 3);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
    }
    setVec3UIntArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec4UIntArray(const std::string& name, const vec4uint* vals, uint count) {
    uniforms[name] = std::vector<vec4uint>(vals, vals+count);
    const auto& vector = this->getVec4UIntArray(name);

    std::vector<uint> flat;
    flat.reserve(count * 4);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
        flat.push_back(std::get<3>(vec));
    }
    setVec4UIntArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setFloatArray(const std::string& name, const float* vals, uint count) {
    uniforms[name] = std::vector<float>(vals, vals+count);
    const auto& vector = this->getFloatArray(name);
    setFloatArrayHelper(shaderProgram, name, vector.data(), count);
}

void Shader::setVec2Array(const std::string& name, const vec2* vals, uint count) {
    uniforms[name] = std::vector<vec2>(vals, vals+count);
    const auto& vector = this->getVec2Array(name);

    std::vector<float> flat;
    flat.reserve(count * 2);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
    }
    setVec2ArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec3Array(const std::string& name, const vec3* vals, uint count) {
    uniforms[name] = std::vector<vec3>(vals, vals+count);
    const auto& vector = this->getVec3Array(name);

    std::vector<float> flat;
    flat.reserve(count * 3);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
    }
    setVec3ArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec4Array(const std::string& name, const vec4* vals, uint count) {
    uniforms[name] = std::vector<vec4>(vals, vals+count);
    const auto& vector = this->getVec4Array(name);

    std::vector<float> flat;
    flat.reserve(count * 4);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
        flat.push_back(std::get<3>(vec));
    }
    setVec4ArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setDoubleArray(const std::string& name, const double* vals, uint count) {
    uniforms[name] = std::vector<double>(vals, vals+count);
    const auto& vector = this->getDoubleArray(name);
    setDoubleArrayHelper(shaderProgram, name, vector.data(), count);
}

void Shader::setVec2DoubleArray(const std::string& name, const vec2double* vals, uint count) {
    uniforms[name] = std::vector<vec2double>(vals, vals+count);
    const auto& vector = this->getVec2DoubleArray(name);

    std::vector<double> flat;
    flat.reserve(count * 2);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
    }
    setVec2DoubleArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec3DoubleArray(const std::string& name, const vec3double* vals, uint count) {
    uniforms[name] = std::vector<vec3double>(vals, vals+count);
    const auto& vector = this->getVec3DoubleArray(name);

    std::vector<double> flat;
    flat.reserve(count * 3);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
    }
    setVec3DoubleArrayHelper(shaderProgram, name, flat.data(), count);
}

void Shader::setVec4DoubleArray(const std::string& name, const vec4double* vals, uint count) {
    uniforms[name] = std::vector<vec4double>(vals, vals+count);
    const auto& vector = this->getVec4DoubleArray(name);

    std::vector<double> flat;
    flat.reserve(count * 4);
    for (auto const& vec : vector) {
        flat.push_back(std::get<0>(vec));
        flat.push_back(std::get<1>(vec));
        flat.push_back(std::get<2>(vec));
        flat.push_back(std::get<3>(vec));
    }
    setVec4DoubleArrayHelper(shaderProgram, name, flat.data(), count);
}


// * Uniform Getters

auto Shader::getInt(const std::string& name) -> int {
    return std::get<int>(uniforms[name]);
}

auto Shader::getVec2Int(const std::string& name) -> vec2int {
    return std::get<vec2int>(uniforms[name]);
}

auto Shader::getVec3Int(const std::string& name) -> vec3int {
    return std::get<vec3int>(uniforms[name]);
}

auto Shader::getVec4Int(const std::string& name) -> vec4int {
    return std::get<vec4int>(uniforms[name]);
}

auto Shader::getUInt(const std::string& name) -> uint {
    return std::get<uint>(uniforms[name]);
}

auto Shader::getVec2UInt(const std::string& name) -> vec2uint {
    return std::get<vec2uint>(uniforms[name]);
}

auto Shader::getVec3UInt(const std::string& name) -> vec3uint {
    return std::get<vec3uint>(uniforms[name]);
}

auto Shader::getVec4UInt(const std::string& name) -> vec4uint {
    return std::get<vec4uint>(uniforms[name]);
}

auto Shader::getFloat(const std::string& name) -> float {
    return std::get<float>(uniforms[name]);
}

auto Shader::getVec2(const std::string& name) -> vec2 {
    return std::get<vec2>(uniforms[name]);
}

auto Shader::getVec3(const std::string& name) -> vec3 {
    return std::get<vec3>(uniforms[name]);
}

auto Shader::getVec4(const std::string& name) -> vec4 {
    return std::get<vec4>(uniforms[name]);
}

auto Shader::getDouble(const std::string& name) -> double {
    return std::get<double>(uniforms[name]);
}

auto Shader::getVec2Double(const std::string& name) -> vec2double {
    return std::get<vec2double>(uniforms[name]);
}

auto Shader::getVec3Double(const std::string& name) -> vec3double {
    return std::get<vec3double>(uniforms[name]);
}

auto Shader::getVec4Double(const std::string& name) -> vec4double {
    return std::get<vec4double>(uniforms[name]);
}

auto Shader::getIntArray(const std::string& name) -> const std::vector<int>& {
    return std::get<std::vector<int>>(uniforms.at(name));
}

auto Shader::getVec2IntArray(const std::string& name) -> const std::vector<vec2int>& {
    return std::get<std::vector<vec2int>>(uniforms.at(name));
}

auto Shader::getVec3IntArray(const std::string& name) -> const std::vector<vec3int>& {
    return std::get<std::vector<vec3int>>(uniforms.at(name));
}

auto Shader::getVec4IntArray(const std::string& name) -> const std::vector<vec4int>& {
    return std::get<std::vector<vec4int>>(uniforms.at(name));
}

auto Shader::getUIntArray(const std::string& name) -> const std::vector<uint>& {
    return std::get<std::vector<uint>>(uniforms.at(name));
}

auto Shader::getVec2UIntArray(const std::string& name) -> const std::vector<vec2uint>& {
    return std::get<std::vector<vec2uint>>(uniforms.at(name));
}

auto Shader::getVec3UIntArray(const std::string& name) -> const std::vector<vec3uint>& {
    return std::get<std::vector<vec3uint>>(uniforms.at(name));
}

auto Shader::getVec4UIntArray(const std::string& name) -> const std::vector<vec4uint>& {
    return std::get<std::vector<vec4uint>>(uniforms.at(name));
}

auto Shader::getFloatArray(const std::string& name) -> const std::vector<float>& {
    return std::get<std::vector<float>>(uniforms.at(name));
}

auto Shader::getVec2Array(const std::string& name) -> const std::vector<vec2>& {
    return std::get<std::vector<vec2>>(uniforms.at(name));
}

auto Shader::getVec3Array(const std::string& name) -> const std::vector<vec3>& {
    return std::get<std::vector<vec3>>(uniforms.at(name));
}

auto Shader::getVec4Array(const std::string& name) -> const std::vector<vec4>& {
    return std::get<std::vector<vec4>>(uniforms.at(name));
}

auto Shader::getDoubleArray(const std::string& name) -> const std::vector<double>& {
    return std::get<std::vector<double>>(uniforms.at(name));
}

auto Shader::getVec2DoubleArray(const std::string& name) -> const std::vector<vec2double>& {
    return std::get<std::vector<vec2double>>(uniforms.at(name));
}

auto Shader::getVec3DoubleArray(const std::string& name) -> const std::vector<vec3double>& {
    return std::get<std::vector<vec3double>>(uniforms.at(name));
}

auto Shader::getVec4DoubleArray(const std::string& name) -> const std::vector<vec4double>& {
    return std::get<std::vector<vec4double>>(uniforms.at(name));
}


// * Other Stuff

void Shader::recompile() {
    deleteFragmentShader();
    compileFragmentShader();
    link();
    use();
    for (const auto& [name, val] : uniforms) {
        if (std::holds_alternative<int>(val))
            setIntHelper(shaderProgram, name, std::get<int>(val));
        else if (std::holds_alternative<vec2int>(val))
            setVec2IntHelper(shaderProgram, name, std::get<vec2int>(val));
        else if (std::holds_alternative<vec3int>(val))
            setVec3IntHelper(shaderProgram, name, std::get<vec3int>(val));
        else if (std::holds_alternative<vec4int>(val))
            setVec4IntHelper(shaderProgram, name, std::get<vec4int>(val));
        else if (std::holds_alternative<uint>(val))
            setUIntHelper(shaderProgram, name, std::get<uint>(val));
        else if (std::holds_alternative<vec2uint>(val))
            setVec2UIntHelper(shaderProgram, name, std::get<vec2uint>(val));
        else if (std::holds_alternative<vec3uint>(val))
            setVec3UIntHelper(shaderProgram, name, std::get<vec3uint>(val));
        else if (std::holds_alternative<vec4uint>(val))
            setVec4UIntHelper(shaderProgram, name, std::get<vec4uint>(val));
        else if (std::holds_alternative<float>(val))
            setFloatHelper(shaderProgram, name, std::get<float>(val));
        else if (std::holds_alternative<vec2>(val))
            setVec2Helper(shaderProgram, name, std::get<vec2>(val));
        else if (std::holds_alternative<vec3>(val))
            setVec3Helper(shaderProgram, name, std::get<vec3>(val));
        else if (std::holds_alternative<vec4>(val))
            setVec4Helper(shaderProgram, name, std::get<vec4>(val));
        else if (std::holds_alternative<double>(val))
            setDoubleHelper(shaderProgram, name, std::get<double>(val));
        else if (std::holds_alternative<vec2double>(val))
            setVec2DoubleHelper(shaderProgram, name, std::get<vec2double>(val));
        else if (std::holds_alternative<vec3double>(val))
            setVec3DoubleHelper(shaderProgram, name, std::get<vec3double>(val));
        else if (std::holds_alternative<vec4double>(val))
            setVec4DoubleHelper(shaderProgram, name, std::get<vec4double>(val));
    }
}

std::string Shader::replaceDefines(const std::string& shaderSource) const {
    std::string shaderSourceWidthDefines = shaderSource;
    for (const auto& pair : defines)
        replaceAll(shaderSourceWidthDefines, pair.first, pair.second);
    return shaderSourceWidthDefines;
}

std::string Shader::loadShaderSourceFromPath(const std::string& shaderSourcePath, unsigned int maxDepth, unsigned int depth) {

    std::string shaderSource = readFileToString(shaderSourcePath.c_str());
    
    std::regex includeRegex("#include\\s*\"([^\"]+)\"");
    
    std::string result;
    std::sregex_iterator it(shaderSource.begin(), shaderSource.end(), includeRegex);
    std::sregex_iterator end;
    
    size_t lastPos = 0;
    for (; it != end; ++it) {
        if (depth == maxDepth) {
            std::cerr << "Reached maximum depth while loading shaders with #includes, maybe there is a cycle?" << std::endl;
            throw std::runtime_error("Reached maximum depth while loading shaders with #includes, maybe there is a cycle?");
        }

        auto match = *it;

        // before this match
        result.append(shaderSource, lastPos, static_cast<size_t>(match.position()) - lastPos);

        // capture group 1 = text inside quotes
        std::string includeFilePath = match[1].str();

        // Computed replacement
        std::string includeShaderSourceCode = loadShaderSourceFromPath(getDirectoryFromFilePath(shaderSourcePath) + includeFilePath, maxDepth, depth + 1);

        // rebuild the include line
        result += includeShaderSourceCode;

        lastPos = static_cast<size_t>(match.position() + match.length());
    }
    // append the rest of the string
    result.append(shaderSource, lastPos, shaderSource.size() - lastPos);

    return result;
}

unsigned int Shader::loadShaderFromSource(GLenum type, const std::string& shaderSource) {
    unsigned int shader = glCreateShader(type);
    const char* shaderSourceCString = shaderSource.c_str();
    glShaderSource(shader, 1, &shaderSourceCString, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader failed to compile:\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int Shader::linkShaderProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "Shader program failed to link: " << infoLog << std::endl;
    }
    return shaderProgram;
}
