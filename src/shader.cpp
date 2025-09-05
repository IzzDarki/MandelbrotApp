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


// * set helpers
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
