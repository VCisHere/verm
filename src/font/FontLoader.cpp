#include "FontLoader.h"

#include <iostream>

CFontLoader::CFontLoader() : m_pFTLib(NULL), m_pFTFace(NULL)
{
}

CFontLoader::~CFontLoader()
{
}

bool CFontLoader::Create(const std::string& strFile)
{
    if (FT_Init_FreeType(&m_pFTLib))
    {
        std::cout << "Fail to init FreeType Library" << std::endl;
        return false;
    }

    if (FT_New_Face(m_pFTLib, strFile.c_str(), 0, &m_pFTFace))
    {
        std::cout << "Fail to load font" << std::endl;
        return false;
    }

    FT_Set_Pixel_Sizes(m_pFTFace, 0, 48);

    return true;
}

void CFontLoader::Destroy()
{
    FT_Done_Face(m_pFTFace);
    FT_Done_FreeType(m_pFTLib);
}

bool CFontLoader::GetData(wchar_t ch, GLYPH& Glyph)
{
    std::unordered_map<wchar_t, GLYPH>::iterator Iter = m_GlyphMap.find(ch);
    if (Iter != m_GlyphMap.end())
    {
        Glyph = Iter->second;
        return true;
    }

    if (FT_Load_Char(m_pFTFace, ch, FT_LOAD_RENDER))
    {
        std::cout << "Failed to load Glyph" << std::endl;
        return false;
    }

    Glyph.dwWidth = m_pFTFace->glyph->bitmap.width;
    Glyph.dwHeight = m_pFTFace->glyph->bitmap.rows;
    Glyph.dwLeft = m_pFTFace->glyph->bitmap_left;
    Glyph.dwTop = m_pFTFace->glyph->bitmap_top;
    Glyph.dwAdvanceX = m_pFTFace->glyph->advance.x;
    Glyph.pData = new char[Glyph.dwWidth * Glyph.dwHeight];
    memcpy(Glyph.pData, m_pFTFace->glyph->bitmap.buffer, Glyph.dwWidth * Glyph.dwHeight);

    m_GlyphMap.insert(std::make_pair(ch, Glyph));

    return true;
}
