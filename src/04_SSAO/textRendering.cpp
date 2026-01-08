#include "textRendering.h"
#include <iostream>

void TextRenderer::InitRenderData()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 6 vertices * 4 floats (x, y, u, v)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

bool TextRenderer::Load(const std::string& fontPath, int pixelSize){

   // Load：初始化 FreeType + 打开字体 + 预加载 ASCII
    Clear();

    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library\n";
        ft = nullptr;
        return false;
    }

    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font: " << fontPath << "\n";
        FT_Done_FreeType(ft);
        ft = nullptr;
        face = nullptr;
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    ready = true;

    // 预加载 ASCII，方便混排
    for (char32_t cp = 0; cp < 128; ++cp) {
        LoadGlyph(cp);
    }

    InitRenderData();
    return true;
}

bool TextRenderer::LoadGlyph(char32_t cp)
{
    if (!ready || !face) return false;
    if (Characters.find(cp) != Characters.end()) return true;

    // 用 Unicode 码点加载字形
    if (FT_Load_Char(face, static_cast<FT_ULong>(cp), FT_LOAD_RENDER)) {
        // 字体里没有这个字，就不缓存
        return false;
    }

    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character ch;
    ch.TextureID = texture;
    ch.Size      = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
    ch.Bearing   = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
    ch.Advance   = static_cast<unsigned int>(face->glyph->advance.x);

    Characters.emplace(cp, ch);
    return true;
}

void TextRenderer::RenderText(Shader& shader,
                              const std::u32string_view& text,
                              float x, float y,
                              float scale,
                              const glm::vec3& color)
{
    if (!ready) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    shader.use();
    shader.setVec3("textColor", color);
    shader.setInt("text", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (char32_t cp : text) {

        // 按需加载：中文第一次出现就生成纹理
        if (Characters.find(cp) == Characters.end()) {
            LoadGlyph(cp);
        }

        auto it = Characters.find(cp);
        if (it == Characters.end()) {
            // 字体缺字：跳过或你可替换成 '?'
            continue;
        }

        const Character& ch = it->second;

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos,     ypos,     0.0f, 1.0f },
            { xpos + w, ypos,     1.0f, 1.0f },

            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos,     1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void TextRenderer::Clear()
{
    // 删纹理
    for (auto& kv : Characters) {
        if (kv.second.TextureID != 0) {
            glDeleteTextures(1, &kv.second.TextureID);
        }
    }
    Characters.clear();

    if (VBO) { glDeleteBuffers(1, &VBO); VBO = 0; }
    if (VAO) { glDeleteVertexArrays(1, &VAO); VAO = 0; }

    // 释放 FreeType（关键）
    if (face) { FT_Done_Face(face); face = nullptr; }
    if (ft)   { FT_Done_FreeType(ft); ft = nullptr; }
    ready = false;
}
