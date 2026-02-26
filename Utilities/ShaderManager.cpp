#include "ShaderManager.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

ShaderManager::ShaderManager() : m_programID(0) {}

ShaderManager::~ShaderManager()
{
    if (m_programID != 0)
    {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
}

std::string ShaderManager::ReadFileToString(const char* path) const
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint ShaderManager::CompileShader(GLenum type, const std::string& source) const
{
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }

    return shader;
}

bool ShaderManager::LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    std::string vsrc = ReadFileToString(vertex_file_path);
    std::string fsrc = ReadFileToString(fragment_file_path);

    if (vsrc.empty() || fsrc.empty())
    {
        return false;
    }

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsrc);

    if (m_programID != 0)
    {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vs);
    glAttachShader(m_programID, fs);
    glLinkProgram(m_programID);

    GLint success = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[2048];
        glGetProgramInfoLog(m_programID, sizeof(log), nullptr, log);
        std::cerr << "Program link error: " << log << std::endl;

        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(m_programID);
        m_programID = 0;
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

void ShaderManager::use() const
{
    glUseProgram(m_programID);
}

void ShaderManager::setIntValue(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
}

void ShaderManager::setFloatValue(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_programID, name.c_str()), value);
}

void ShaderManager::setVec2Value(const std::string& name, const glm::vec2& vec) const
{
    glUniform2fv(glGetUniformLocation(m_programID, name.c_str()), 1, glm::value_ptr(vec));
}

void ShaderManager::setVec3Value(const std::string& name, const glm::vec3& vec) const
{
    glUniform3fv(glGetUniformLocation(m_programID, name.c_str()), 1, glm::value_ptr(vec));
}

void ShaderManager::setVec4Value(const std::string& name, const glm::vec4& vec) const
{
    glUniform4fv(glGetUniformLocation(m_programID, name.c_str()), 1, glm::value_ptr(vec));
}

void ShaderManager::setMat4Value(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void ShaderManager::setSampler2DValue(const std::string& name, int slot) const
{
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), slot);
}