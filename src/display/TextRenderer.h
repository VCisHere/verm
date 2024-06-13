#ifndef _TEXT_RENDERER_H_
#define _TEXT_RENDERER_H_

#include "glad/glad.h"

#include "glm.hpp"

#include <string>

class CShader;
class CFontLoader;

class CTextRenderer
{
public:
    CTextRenderer();
    ~CTextRenderer();

    bool Create();
    void Destroy();

    void SetWindowSize(uint32_t dwWidth, uint32_t dwHeight);

    // todo: type
    void RenderText(const std::wstring& str, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);

private:
    CShader* m_pShader;

    CFontLoader* m_pFontLoader;

    GLuint m_VAO; // todo
    GLuint m_VBO;
};

#endif // _TEXT_RENDERER_H_
