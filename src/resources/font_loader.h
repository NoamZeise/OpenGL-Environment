#ifndef GLFONT_LOADER_H
#define GLFONT_LOADER_H

#include <string.h>
#include <map>
#include <vector>

#include <glm/glm.hpp>

#include "texture_loader.h"

struct FontData;

namespace Resource {

class GLFontLoader {
public:
    GLFontLoader() {};
    ~GLFontLoader();
    void UnloadFonts();
    Font LoadFont(std::string file, GLTextureLoader* texLoader);
    std::vector<QuadDraw> DrawString(Font drawfont, std::string text, glm::vec2 position,
				     float size, float depth, glm::vec4 colour, float rotate);
    float MeasureString(Font font, std::string text, float size);
    
private:
    std::vector<FontData*> fonts;
};
} // namespace Resource

#endif
