#include "Shader.h"

#include "glad/glad.h"

void CheckShaderCompileErrors(uint32_t dwShaderID)
{
    int nSuccess = 0;
    char szInfoLog[1024];
    glGetShaderiv(dwShaderID, GL_COMPILE_STATUS, &nSuccess);
    if (!nSuccess)
    {
        glGetShaderInfoLog(dwShaderID, sizeof(szInfoLog), NULL, szInfoLog);
        std::cout << "Fail to compile shader: " << szInfoLog << std::endl;
    }
}

void CheckProgramCompileErrors(uint32_t dwShaderID)
{
    int nSuccess = 0;
    char szInfoLog[1024];
    glGetProgramiv(dwShaderID, GL_LINK_STATUS, &nSuccess);
    if (!nSuccess)
    {
        glGetProgramInfoLog(dwShaderID, sizeof(szInfoLog), NULL, szInfoLog);
        std::cout << "Fail to link program " << szInfoLog << std::endl;
    }
}

CShader::CShader(const std::string& strVertPath, const std::string& strFragPath)
{
    std::string strVertShaderCode;
    std::string strFragShaderCode;
    std::ifstream vertShaderFile;
    std::ifstream fragShaderFile;
    vertShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        vertShaderFile.open(strVertPath);
        fragShaderFile.open(strFragPath);
        std::stringstream vertShaderStream;
        std::stringstream fragShaderStream;

        vertShaderStream << vertShaderFile.rdbuf();
        fragShaderStream << fragShaderFile.rdbuf();

        vertShaderFile.close();
        fragShaderFile.close();

        strVertShaderCode = vertShaderStream.str();
        strFragShaderCode = fragShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
    }

    const char* szVertShaderCode = strVertShaderCode.c_str();
    const char* szFragShaderCode = strFragShaderCode.c_str();

    // vertex shader
    uint32_t dwVertShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(dwVertShaderID, 1, &szVertShaderCode, NULL);
    glCompileShader(dwVertShaderID);
    CheckShaderCompileErrors(dwVertShaderID);

    // fragment Shader
    uint32_t dwFragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(dwFragShaderID, 1, &szFragShaderCode, NULL);
    glCompileShader(dwFragShaderID);
    CheckShaderCompileErrors(dwFragShaderID);

    // shader Program
    m_dwID = glCreateProgram();
    glAttachShader(m_dwID, dwVertShaderID);
    glAttachShader(m_dwID, dwFragShaderID);
    glLinkProgram(m_dwID);
    CheckProgramCompileErrors(m_dwID);

    glDeleteShader(dwVertShaderID);
    glDeleteShader(dwFragShaderID);
}

CShader::~CShader()
{
}

void CShader::Use()
{
    glUseProgram(m_dwID);
}

uint32_t CShader::GetID()
{
    return m_dwID;
}

void CShader::SetBool(const std::string& strName, bool bVal) const
{
    glUniform1i(glGetUniformLocation(m_dwID, strName.c_str()), (int)bVal);
}

void CShader::SetInt(const std::string& strName, int nVal) const
{
    glUniform1i(glGetUniformLocation(m_dwID, strName.c_str()), nVal);
}

void CShader::SetFloat(const std::string& strName, float fVal) const
{
    glUniform1f(glGetUniformLocation(m_dwID, strName.c_str()), fVal);
}
