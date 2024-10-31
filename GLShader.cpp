#include "GLShader.h"
#include <GL/glew.h>
#include <GL/gl.h>

GLShader::GLShader() : m_Program(0) {}

GLShader::~GLShader() {
    Destroy();
}

std::string GLShader::ReadFile(const char* filePath) {
    std::ifstream shaderFile(filePath, std::ios::in);
    if (!shaderFile.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }

    std::string shaderCode((std::istreambuf_iterator<char>(shaderFile)),
        std::istreambuf_iterator<char>());
    shaderFile.close();
    return shaderCode;
}

bool GLShader::CompileShader(const char* shaderCode, uint32_t shaderType, uint32_t& shaderID) {
    shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderCode, nullptr);
    glCompileShader(shaderID);

    int success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderID, 512, nullptr, infoLog);
        std::cerr << "Error compiling shader: " << infoLog << std::endl;
        return false;
    }
    return true;
}

bool GLShader::LoadShaders(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = ReadFile(vertexPath);
    std::string fragmentCode = ReadFile(fragmentPath);
    if (vertexCode.empty() || fragmentCode.empty()) return false;

    uint32_t vertexShader, fragmentShader;
    if (!CompileShader(vertexCode.c_str(), GL_VERTEX_SHADER, vertexShader) ||
        !CompileShader(fragmentCode.c_str(), GL_FRAGMENT_SHADER, fragmentShader)) {
        return false;
    }

    m_Program = glCreateProgram();
    glAttachShader(m_Program, vertexShader);
    glAttachShader(m_Program, fragmentShader);
    glLinkProgram(m_Program);

    int success;
    glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_Program, 512, nullptr, infoLog);
        std::cerr << "Error linking program: " << infoLog << std::endl;
        Destroy();
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void GLShader::Use() const {
    glUseProgram(m_Program);
}

void GLShader::Destroy() {
    if (m_Program) {
        glDeleteProgram(m_Program);
        m_Program = 0;
    }
}
