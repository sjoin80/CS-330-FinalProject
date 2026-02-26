#pragma once

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    bool LoadShaders(const char* vertex_file_path, const char* fragment_file_path);
    void use() const;

    void setMat4Value(const std::string& name, const glm::mat4& mat) const;
    void setVec4Value(const std::string& name, const glm::vec4& vec) const;
    void setVec3Value(const std::string& name, const glm::vec3& vec) const;
    void setVec2Value(const std::string& name, const glm::vec2& vec) const;
    void setFloatValue(const std::string& name, float value) const;
    void setIntValue(const std::string& name, int value) const;
    void setSampler2DValue(const std::string& name, int slot) const;

    unsigned int ID() const { return m_programID; }

private:
    unsigned int m_programID = 0;

    std::string ReadFileToString(const char* path) const;
    GLuint CompileShader(GLenum type, const std::string& source) const;
};