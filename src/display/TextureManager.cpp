#include "TextureManager.h"
#include "FontLoader.h"

#include "glad/glad.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

CTextureManager::CTextureManager() : m_pFontLoader(nullptr)
{
}

CTextureManager::~CTextureManager()
{
}

bool CTextureManager::Create()
{
    m_pFontLoader = new CFontLoader();
    if (!m_pFontLoader->Create("./res/font/CascadiaMono.ttf"))
    {
        delete m_pFontLoader;
        m_pFontLoader = nullptr;
        return false;
    }

    return true;
}

void CTextureManager::Destroy()
{
    m_pFontLoader->Destroy();
    delete m_pFontLoader;
    m_pFontLoader = nullptr;
}

bool CTextureManager::GetTextTexture(char32_t code, CHARACTOR& Charactor)
{
    std::unordered_map<char32_t, CHARACTOR>::iterator Iter = m_CharactorMap.find(code);
    if (Iter != m_CharactorMap.end())
    {
        Charactor = Iter->second;
        return true;
    }

    CFontLoader::GLYPH Glyph;
    if (!m_pFontLoader->GetData(code, Glyph))
    {
        return false;
    }

    uint32_t dwTextureID = 0;
    glGenTextures(1, &dwTextureID);
    glBindTexture(GL_TEXTURE_2D, dwTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Glyph.dwWidth, Glyph.dwHeight, 0, GL_RED, GL_UNSIGNED_BYTE, Glyph.pData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Charactor.dwTextureID = dwTextureID;
    Charactor.Size = glm::ivec2(Glyph.dwWidth, Glyph.dwHeight);
    Charactor.Bearing = glm::ivec2(Glyph.dwLeft, Glyph.dwTop);
    Charactor.dwAdvance = Glyph.dwAdvanceX;

    m_CharactorMap.insert(std::make_pair(code, Charactor));

    return true;
}
