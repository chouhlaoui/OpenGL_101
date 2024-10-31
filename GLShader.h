#pragma once

#include <cstdint>
#include <string>
#include <fstream>
#include <iostream>

class GLShader {
public:
    uint32_t m_Program;

    GLShader();
    ~GLShader();

    uint32_t GetProgram() const { return m_Program; }

    bool LoadShaders(const char* vertexPath, const char* fragmentPath);
    void Use() const;
    void Destroy();

private:
    bool CompileShader(const char* shaderCode, uint32_t shaderType, uint32_t& shaderID);
    std::string ReadFile(const char* filePath);
};
