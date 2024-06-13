#include "TextRenderer.h"
#include "FontLoader.h"
#include "Shader.h"

#include "glad/glad.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

struct Character
{
    GLuint TextureID;   // 字形纹理的ID
    glm::ivec2 Size;    // 字形大小
    glm::ivec2 Bearing; // 从基准线到字形左部/顶部的偏移值
    GLuint Advance;     // 原点距下一个字形原点的距离
};

CTextRenderer::CTextRenderer() : m_pShader(nullptr), m_pFontLoader(nullptr), m_VAO(0), m_VBO(0)
{
}

CTextRenderer::~CTextRenderer()
{
}

bool CTextRenderer::Create()
{
    m_pShader = new CShader("./res/shader/text.vs", "./res/shader/text.fs");
    m_pShader->Use();

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_pFontLoader = new CFontLoader();
    m_pFontLoader->Create("./res/font/simsun.ttc");

    return true;
}

void CTextRenderer::Destroy()
{
    // todo
}

void CTextRenderer::SetWindowSize(uint32_t dwWidth, uint32_t dwHeight)
{
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(dwWidth), 0.0f, static_cast<float>(dwHeight));
    glUniformMatrix4fv(glGetUniformLocation(m_pShader->GetID(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void CTextRenderer::RenderText(const std::wstring& str, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    m_pShader->Use();
    glUniform3f(glGetUniformLocation(m_pShader->GetID(), "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_VAO);

    std::wstring::const_iterator c;
    for (c = str.begin(); c != str.end(); c++)
    {
        CFontLoader::GLYPH Glyph;
        m_pFontLoader->GetData(*c, Glyph);

        uint32_t dwTextureID;
        glGenTextures(1, &dwTextureID);
        glBindTexture(GL_TEXTURE_2D, dwTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Glyph.dwWidth, Glyph.dwHeight, 0, GL_RED, GL_UNSIGNED_BYTE, Glyph.pData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch = {dwTextureID, glm::ivec2(Glyph.dwWidth, Glyph.dwHeight), glm::ivec2(Glyph.dwLeft, Glyph.dwTop),
                        static_cast<unsigned int>(Glyph.dwAdvanceX)};

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat vertices[6][4] = {{xpos, ypos + h, 0.0, 0.0}, {xpos, ypos, 0.0, 1.0},
                                  {xpos + w, ypos, 1.0, 1.0}, {xpos, ypos + h, 0.0, 0.0},
                                  {xpos + w, ypos, 1.0, 1.0}, {xpos + w, ypos + h, 1.0, 0.0}};

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale; // 2 << 6 = 64
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
