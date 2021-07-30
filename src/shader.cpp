#include "shader.h"

Shader::Shader(const std::string& vertexShaderSourcePath, const std::string& fragmentShaderSourcePath, bool compileAndLink, bool clean) {
    vertexShaderSource = readFileToString(vertexShaderSourcePath.c_str());
    fragmentShaderSource = readFileToString(fragmentShaderSourcePath.c_str());

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
    vertexShader = loadShaderFromFile(GL_VERTEX_SHADER, finalShaderSource);
}

void Shader::compileFragmentShader() {
    std::string finalShaderSource = replaceDefines(fragmentShaderSource);
    fragmentShader = loadShaderFromFile(GL_FRAGMENT_SHADER, finalShaderSource);
}

void Shader::link() {
    shaderProgram = linkShaderProgram(vertexShader, fragmentShader);
}

void Shader::use() const {
    glUseProgram(shaderProgram);
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setVec2Int(const std::string& name, int x, int y) {
    glUniform2i(glGetUniformLocation(shaderProgram, name.c_str()), x, y);
}

void Shader::setVec3Int(const std::string& name, int x, int y, int z) {
    glUniform3i(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}

void Shader::setVec4Int(const std::string& name, int x, int y, int z, int w) {
    glUniform4i(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z, w);
}

void Shader::setUInt(const std::string& name, unsigned int value) {
    glUniform1ui(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setVec2UInt(const std::string& name, unsigned int x, unsigned int y) {
    glUniform2ui(glGetUniformLocation(shaderProgram, name.c_str()), x, y);
}

void Shader::setVec3UInt(const std::string& name, unsigned int x, unsigned int y, unsigned int z) {
    glUniform3ui(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}

void Shader::setVec4UInt(const std::string& name, unsigned int x, unsigned int y, unsigned int z, unsigned int w) {
    glUniform4ui(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z, w);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, float x, float y) {
    glUniform2f(glGetUniformLocation(shaderProgram, name.c_str()), x, y);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z, w);
}

void Shader::setDouble(const std::string& name, double value) {
    glUniform1d(glGetUniformLocation(shaderProgram, name.c_str()), value);
}

void Shader::setVec2Double(const std::string& name, double x, double y) {
    glUniform2d(glGetUniformLocation(shaderProgram, name.c_str()), x, y);
}

void Shader::setVec3Double(const std::string& name, double x, double y, double z) {
    glUniform3d(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z);
}

void Shader::setVec4Double(const std::string& name, double x, double y, double z, double w) {
    glUniform4d(glGetUniformLocation(shaderProgram, name.c_str()), x, y, z, w);
}

void Shader::mandelRecompileWithColor(int colorNumber) {
    define("FLOW_COLOR_TYPE", std::to_string(colorNumber));
    deleteFragmentShader();
    compileFragmentShader();
    link();
}

std::string Shader::replaceDefines(const std::string& shaderSource) const {
    std::string shaderSourceWidthDefines = shaderSource;
    for (const auto& pair : defines)
        replaceAll(shaderSourceWidthDefines, pair.first, pair.second);
    return shaderSourceWidthDefines;
}

unsigned int Shader::loadShaderFromFile(int type, const std::string& shaderSource) {
    unsigned int shader = glCreateShader(type);
    const char* shaderSourceCString = shaderSource.c_str();
    glShaderSource(shader, 1, &shaderSourceCString, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader failed to compile: " << infoLog << std::endl;
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
