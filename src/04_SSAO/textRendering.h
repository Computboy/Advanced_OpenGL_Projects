#pragma once

#include <map>
#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// 使用 FreeType 进行文本渲染的Character结构体
struct Character{
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class TextRenderer {
public:
    TextRenderer() = default;
    ~TextRenderer() { Clear(); }

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    // 禁用拷贝

    bool Load(const std::string& fontPath, int pixelSize);

    // 渲染字符串（屏幕空间像素坐标）
    // shader 需要有：uniform mat4 projection; uniform vec3 textColor; uniform sampler2D text;
    void RenderText(Shader& shader,
                    const std::u32string_view& text,
                    float x, float y,
                    float scale,
                    const glm::vec3& color);

    // 清理所有 glyph texture + VAO/VBO
    void Clear();

    FT_Library ft = nullptr;
    FT_Face face = nullptr;
    bool ready = false;

    // 可选：外部读缓存
    const std::map<char32_t, Character>& GetCharacters() const { return Characters; }

private:
    std::map<char32_t, Character> Characters;

    GLuint VAO = 0;
    GLuint VBO = 0;

private:
    void InitRenderData();
    bool LoadGlyph(char32_t cp);
};