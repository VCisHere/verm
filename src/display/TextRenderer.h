#ifndef _TEXT_RENDERER_H_
#define _TEXT_RENDERER_H_

#include "glad/glad.h"

#include "glm.hpp"

#include <string>

class CShader;
class CFontLoader;
class CTextureManager;

class CTextRenderer
{
public:
    CTextRenderer();
    ~CTextRenderer();

    bool Create();
    void Destroy();

    void SetWindowSize(uint32_t dwWidth, uint32_t dwHeight);

    // todo: type
    bool Render(uint32_t dwColumn, uint32_t dwRow, char32_t code, glm::vec3 color);

    // todo: render string

private:
    CShader* m_pShader;

    CTextureManager* m_pTextureManager;

    GLuint m_VAO; // todo
    GLuint m_VBO;
};

#endif // _TEXT_RENDERER_H_
