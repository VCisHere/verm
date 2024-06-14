#ifndef _TEXT_MANAGER_H_
#define _TEXT_MANAGER_H_

#include "glm.hpp"

#include <unordered_map>

class CFontLoader;

class CTextureManager
{
public:
    typedef struct _tag_charactor
    {
        uint32_t dwTextureID; // 字形纹理的ID
        glm::ivec2 Size;      // 字形大小
        glm::ivec2 Bearing;   // 从基准线到字形左部/顶部的偏移值
        uint32_t dwAdvance;   // 原点距下一个字形原点的距离
    } CHARACTOR;

public:
    CTextureManager();
    ~CTextureManager();

    bool Create();
    void Destroy();

    bool GetTextTexture(char32_t code, CHARACTOR& Charactor);

private:
    std::unordered_map<char32_t, CHARACTOR> m_CharactorMap;

    CFontLoader* m_pFontLoader;
};

#endif // _TEXT_MANAGER_H_
