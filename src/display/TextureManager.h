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
        uint32_t dwTextureID; // ���������ID
        glm::ivec2 Size;      // ���δ�С
        glm::ivec2 Bearing;   // �ӻ�׼�ߵ�������/������ƫ��ֵ
        uint32_t dwAdvance;   // ԭ�����һ������ԭ��ľ���
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
