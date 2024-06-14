#ifndef _FONT_LOADER_H_
#define _FONT_LOADER_H_

#include "ft2build.h"
#include FT_FREETYPE_H
// #include FT_GLYPH_H

#include <string>
#include <unordered_map>
#include <vector>

class CFontLoader
{
public:
    typedef struct _tag_bitmap
    {
        char* pData;
        uint32_t dwWidth;
        uint32_t dwHeight;
        uint32_t dwLeft;
        uint32_t dwTop;
    } GLYPH;

    CFontLoader();
    ~CFontLoader();

    bool Create(const std::vector<std::string>& strFileVec);
    void Destroy();

    bool GetData(char32_t ch, GLYPH& Glyph);

private:
    FT_Library m_pFTLib;
    std::vector<FT_Face> m_pFTFaceVec;

    std::unordered_map<char32_t, GLYPH> m_GlyphMap;
};

#endif // _FONT_LOADER_H_
