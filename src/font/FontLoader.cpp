#include "FontLoader.h"

#include <iostream>

CFontLoader::CFontLoader() : m_pFTLib(nullptr)
{
}

CFontLoader::~CFontLoader()
{
}

bool CFontLoader::Create(const std::vector<std::string>& strFileVec)
{
    if (FT_Init_FreeType(&m_pFTLib))
    {
        std::cout << "Fail to init FreeType Library" << std::endl;
        return false;
    }

    m_pFTFaceVec.reserve(strFileVec.size());
    for (size_t i = 0; i < strFileVec.size(); ++i)
    {
        FT_Face pFace = nullptr;
        if (FT_New_Face(m_pFTLib, strFileVec[i].c_str(), 0, &pFace))
        {
            std::cout << "Fail to load font" << std::endl;
            return false;
        }

        // todo: dpi
        FT_Set_Char_Size(pFace, 0, 12 * 64, 96, 0);

        // todo: select charmap
        FT_Select_Charmap(pFace, FT_ENCODING_UNICODE);

        m_pFTFaceVec.push_back(pFace);
    }

    return true;
}

void CFontLoader::Destroy()
{
    for (size_t i = 0; i < m_pFTFaceVec.size(); ++i)
    {
        FT_Done_Face(m_pFTFaceVec[i]);
    }

    FT_Done_FreeType(m_pFTLib);
}

bool CFontLoader::GetData(char32_t ch, GLYPH& Glyph)
{
    std::unordered_map<char32_t, GLYPH>::iterator Iter = m_GlyphMap.find(ch);
    if (Iter != m_GlyphMap.end())
    {
        Glyph = Iter->second;
        return true;
    }

    FT_Face pFace = nullptr;
    for (std::vector<FT_Face>::iterator Iter = m_pFTFaceVec.begin(); Iter != m_pFTFaceVec.end(); ++Iter)
    {
        uint32_t id = FT_Get_Char_Index(*Iter, ch);
        if (id > 0)
        {
            pFace = *Iter;
            break;
        }
    }

    if (pFace == nullptr)
    {
        return false;
    }

    int32_t nFlags =
        FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT | FT_OUTLINE_HIGH_PRECISION | FT_LOAD_NO_BITMAP;
    // int32_t nFlags =
    //     FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT | FT_OUTLINE_HIGH_PRECISION | FT_LOAD_MONOCHROME;
    if (FT_Load_Char(pFace, ch, nFlags))
    {
        std::cout << "Failed to load Glyph" << std::endl;
        return false;
    }

    Glyph.dwWidth = pFace->glyph->bitmap.width;
    Glyph.dwHeight = pFace->glyph->bitmap.rows;
    Glyph.dwLeft = pFace->glyph->bitmap_left;
    Glyph.dwTop = pFace->glyph->bitmap_top;
    Glyph.pData = new char[Glyph.dwWidth * Glyph.dwHeight];
    // size_t i = 0;
    // for (size_t y = 0; y < Glyph.dwHeight; ++y)
    //{
    //     for (size_t x = 0; x < Glyph.dwWidth; ++x)
    //     {
    //         Glyph.pData[i] =
    //             ((pFace->glyph->bitmap.buffer[y * pFace->glyph->bitmap.pitch + x / 8] << (x % 8)) & 0x80) ? 0xFF : 0;
    //     }
    // }
    memcpy(Glyph.pData, pFace->glyph->bitmap.buffer, static_cast<size_t>(Glyph.dwWidth) * Glyph.dwHeight);

    m_GlyphMap.insert(std::make_pair(ch, Glyph));

    return true;
}
