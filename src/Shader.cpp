#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>

Shader::~Shader() {
    if (programID != 0) {
        glDeleteProgram(programID);
    }
}

bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    // Read vertex shader
    std::string vertexCode;
    std::ifstream vShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        std::stringstream vShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        vShaderFile.close();
        vertexCode = vShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        return false;
    }

    // Read fragment shader
    std::string fragmentCode;
    std::ifstream fShaderFile;
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        fShaderFile.open(fragmentPath);
        std::stringstream fShaderStream;
        fShaderStream << fShaderFile.rdbuf();
        fShaderFile.close();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        return false;
    }

    return loadFromString(vertexCode, fragmentCode);
}

bool Shader::loadFromString(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vertexShader, fragmentShader;
    
    // Compile vertex shader
    if (!compileShader(vertexShader, vertexSource, GL_VERTEX_SHADER)) {
        return false;
    }

    // Compile fragment shader
    if (!compileShader(fragmentShader, fragmentSource, GL_FRAGMENT_SHADER)) {
        glDeleteShader(vertexShader);
        return false;
    }

    // Create shader program
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    
    if (!linkProgram(programID)) {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    // Delete shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void Shader::use() const {
    glUseProgram(programID);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}

bool Shader::compileShader(unsigned int& shader, const std::string& source, unsigned int type) {
    shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

bool Shader::linkProgram(unsigned int program) {
    glLinkProgram(program);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }
    return true;
}

int Shader::getUniformLocation(const std::string& name) const {
    int location = glGetUniformLocation(programID, name.c_str());
    if (location == -1) {
        std::cerr << "WARNING::SHADER::UNIFORM_NOT_FOUND: " << name << std::endl;
    }
    return location;
} 