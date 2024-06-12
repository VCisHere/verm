#ifndef _SHADER_H_
#define _SHADER_H_

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class CShader
{
public:
    CShader(const std::string& strVertPath, const std::string& strFragPath);
    ~CShader();

    void Use();

    uint32_t GetID();

    void SetBool(const std::string& strName, bool bVal) const;
    void SetInt(const std::string& strName, int nVal) const;
    void SetFloat(const std::string& strName, float fVal) const;

private:
    uint32_t m_dwID;
};

#endif // _SHADER_H_
